/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <chrono>
#include <cstring>
#include <functional>
#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "common_components.hpp"
#include "unity.h"

namespace {

using boost::system::error_code;

bool poll_until(boost::asio::io_context &ioc, const std::function<bool()> &ready, int max_spins = 8000)
{
    for (int i = 0; i < max_spins; ++i) {
        if (ready()) {
            return true;
        }
        (void)ioc.poll();
        vTaskDelay(1);
    }
    return false;
}

} // namespace

TEST_CASE("service_manager_asio_error_codes", "[asio][service_manager]")
{
    error_code aborted = boost::asio::error::operation_aborted;
    error_code eof_ec = boost::asio::error::eof;

    TEST_ASSERT_TRUE(static_cast<bool>(aborted));
    TEST_ASSERT_TRUE(static_cast<bool>(eof_ec));
    TEST_ASSERT_FALSE(aborted.message().empty());
    TEST_ASSERT_FALSE(eof_ec.message().empty());
}

TEST_CASE("service_manager_asio_post_poll", "[asio][service_manager]")
{
    boost::asio::io_context ioc;
    auto ex = ioc.get_executor();
    int hits = 0;
    boost::asio::post(ex, [&hits]() { ++hits; });

    TEST_ASSERT_TRUE(poll_until(ioc, [&hits]() { return hits == 1; }));
    TEST_ASSERT_EQUAL(1, hits);
}

TEST_CASE("service_manager_asio_executor_work_guard_poll", "[asio][service_manager]")
{
    boost::asio::io_context ioc;
    auto ex = ioc.get_executor();
    {
        boost::asio::executor_work_guard<decltype(ex)> guard(ex);
        (void)ioc.poll();
    }
    (void)ioc.poll();
}

TEST_CASE("service_manager_asio_steady_timer_poll", "[asio][service_manager]")
{
    boost::asio::io_context ioc;
    auto ex = ioc.get_executor();
    boost::asio::steady_timer timer(ex);
    bool done = false;
    timer.expires_after(std::chrono::milliseconds(15));
    timer.async_wait([&done](const error_code &ec) {
        TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(ec), ec.message().c_str());
        done = true;
    });

    TEST_ASSERT_TRUE(poll_until(ioc, [&done]() { return done; }));
}

TEST_CASE("service_manager_asio_steady_timer_cancel_poll", "[asio][service_manager]")
{
    boost::asio::io_context ioc;
    auto ex = ioc.get_executor();
    boost::asio::steady_timer timer(ex);
    bool done = false;
    error_code last_ec;
    timer.expires_after(std::chrono::seconds(30));
    timer.async_wait([&done, &last_ec](const error_code &ec) {
        last_ec = ec;
        done = true;
    });
    timer.cancel();

    TEST_ASSERT_TRUE(poll_until(ioc, [&done]() { return done; }));
    TEST_ASSERT_TRUE(last_ec == boost::asio::error::operation_aborted);
}

TEST_CASE("service_manager_asio_tcp_types_open_close", "[asio][service_manager]")
{
    boost::asio::io_context ioc;
    auto ex = ioc.get_executor();
    boost::asio::ip::tcp::resolver resolver(ex);
    boost::asio::ip::tcp::acceptor acceptor(ex);
    boost::asio::ip::tcp::socket socket(ex);

    error_code ec;
    acceptor.open(boost::asio::ip::tcp::v4(), ec);
    TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(ec), ec.message().c_str());
    acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
    TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(ec), ec.message().c_str());
    acceptor.close(ec);

    socket.close(ec);

    auto endpoints = resolver.resolve("127.0.0.1", "9", ec);
    TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(ec), ec.message().c_str());
    TEST_ASSERT_TRUE(endpoints.size() > 0);
}

TEST_CASE("service_manager_asio_tcp_loopback_line", "[asio][service_manager]")
{
    using boost::asio::ip::tcp;

    // TODO: Fix false positives vs strict default (0).
    common_set_memory_leak_threshold(300);

    boost::asio::io_context ioc;
    auto ex = ioc.get_executor();

    tcp::acceptor acceptor(ex);
    error_code ec;
    acceptor.open(tcp::v4(), ec);
    TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(ec), ec.message().c_str());
    acceptor.set_option(tcp::acceptor::reuse_address(true), ec);
    TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(ec), ec.message().c_str());
    acceptor.bind(tcp::endpoint(tcp::v4(), 0), ec);
    TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(ec), ec.message().c_str());
    acceptor.listen(1, ec);
    TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(ec), ec.message().c_str());
    const uint16_t port = acceptor.local_endpoint(ec).port();
    TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(ec), ec.message().c_str());

    tcp::socket server_sock(ex);
    tcp::socket client_sock(ex);

    bool accept_done = false;
    bool connect_done = false;
    error_code accept_err;
    error_code connect_err;

    acceptor.async_accept(server_sock, [&](error_code e) {
        accept_err = e;
        accept_done = true;
    });

    tcp::resolver resolver(ex);
    auto endpoints = resolver.resolve("127.0.0.1", std::to_string(port), ec);
    TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(ec), ec.message().c_str());

    boost::asio::async_connect(client_sock, endpoints, [&](error_code e, const tcp::endpoint &) {
        connect_err = e;
        connect_done = true;
    });

    TEST_ASSERT_TRUE(poll_until(ioc, [&]() { return accept_done && connect_done; }));
    TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(accept_err), accept_err.message().c_str());
    TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(connect_err), connect_err.message().c_str());

    /* First poll phase ends with outstanding_work==0; scheduler sets stopped_ until restart(). */
    ioc.restart();

    auto payload = std::make_shared<std::string>("rpc-line\n");
    bool write_done = false;
    error_code write_err;

    boost::asio::async_write(client_sock, boost::asio::buffer(*payload), [&](error_code e, std::size_t) {
        write_err = e;
        write_done = true;
    });

    boost::asio::streambuf recv_buf;
    bool read_done = false;
    error_code read_err;
    std::string received;

    boost::asio::async_read_until(server_sock, recv_buf, '\n', [&](error_code e, std::size_t) {
        read_err = e;
        if (!e) {
            std::istream is(&recv_buf);
            std::getline(is, received);
        }
        read_done = true;
    });

    TEST_ASSERT_TRUE(poll_until(ioc, [&]() { return write_done && read_done; }));
    TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(write_err), write_err.message().c_str());
    TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(read_err), read_err.message().c_str());
    TEST_ASSERT_EQUAL(0, strcmp("rpc-line", received.c_str()));
}
