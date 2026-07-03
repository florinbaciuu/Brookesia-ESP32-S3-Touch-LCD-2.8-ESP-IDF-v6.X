# Git Bible

## Ghid practic pentru dezvoltatori

> Scop: să înțelegi Git, nu doar să memorezi comenzi.

------------------------------------------------------------------------

# 1. Modelul Git

Git lucrează cu trei zone:

    Working Directory
          │
          ▼
     Staging Area (Index)
          │
          ▼
     Repository (.git)

-   **Working Directory** -- fișierele pe care le modifici.
-   **Staging Area** -- modificările pregătite pentru commit.
-   **Repository** -- istoricul commit-urilor.

Fluxul normal:

``` bash
git add .
git commit -m "Mesaj"
git push
```

------------------------------------------------------------------------

# 2. Verificare

``` bash
git status
git branch
git branch --show-current
git branch -a
git branch -r
git log --oneline --graph --decorate --all
```

------------------------------------------------------------------------

# 3. Branch-uri

Creare:

``` bash
git switch -c feature/audio
```

Schimbare:

``` bash
git switch main
```

Ștergere:

``` bash
git branch -d feature/audio
git branch -D feature/audio
git push origin --delete feature/audio
```

------------------------------------------------------------------------

# 4. Commit-uri

``` bash
git add .
git commit -m "Added audio player"
```

Vezi istoricul:

``` bash
git log
git log --oneline
```

Ultimele modificări:

``` bash
git show
```

------------------------------------------------------------------------

# 5. Remote

Vezi remote:

``` bash
git remote -v
```

Prima împingere:

``` bash
git push -u origin feature/audio
```

Ulterior:

``` bash
git push
git pull
```

------------------------------------------------------------------------

# 6. Merge

Mută-te pe branch-ul destinație:

``` bash
git switch main
```

Apoi:

``` bash
git merge feature/audio
```

------------------------------------------------------------------------

# 7. Rebase

Reaplică commit-urile peste ultimul `main`.

``` bash
git switch feature/audio
git fetch
git rebase origin/main
```

**Merge** păstrează istoricul ramificat.

**Rebase** produce un istoric liniar.

------------------------------------------------------------------------

# 8. Stash

Salvează modificări neterminate:

``` bash
git stash
```

Revino:

``` bash
git stash pop
```

Vezi lista:

``` bash
git stash list
```

------------------------------------------------------------------------

# 9. Reset

Doar mută HEAD:

``` bash
git reset --soft HEAD~1
```

Anulează staging:

``` bash
git reset --mixed HEAD~1
```

Șterge complet modificările:

``` bash
git reset --hard HEAD~1
```

⚠️ `--hard` poate duce la pierderea modificărilor.

------------------------------------------------------------------------

# 10. Revert

Anulează un commit fără a rescrie istoricul:

``` bash
git revert <commit>
```

Ideal pentru proiecte partajate.

------------------------------------------------------------------------

# 11. Reflog

Cea mai utilă comandă când ai făcut o greșeală.

``` bash
git reflog
```

Poți reveni la orice stare:

``` bash
git reset --hard HEAD@{3}
```

------------------------------------------------------------------------

# 12. Cherry-pick

Copiază un singur commit:

``` bash
git cherry-pick <hash>
```

------------------------------------------------------------------------

# 13. Tag-uri

``` bash
git tag v1.0
git push origin v1.0
```

------------------------------------------------------------------------

# 14. Conflicte

După conflict:

``` bash
git status
```

Editezi fișierele, apoi:

``` bash
git add .
git commit
```

------------------------------------------------------------------------

# 15. Ignorarea fișierelor

`.gitignore`

Exemplu:

``` text
build/
.vscode/
*.o
*.obj
*.exe
sdkconfig.old
```

------------------------------------------------------------------------

# 16. Alias-uri utile

``` bash
git config --global alias.st status
git config --global alias.co switch
git config --global alias.br branch
git config --global alias.lg "log --oneline --graph --decorate --all"
```

------------------------------------------------------------------------

# 17. Workflow recomandat

``` bash
git switch main
git pull

git switch -c feature/nume

# modificări

git add .
git commit -m "Descriere"

git push -u origin feature/nume

# după merge

git switch main
git pull

git branch -D feature/nume
git fetch --prune
```

------------------------------------------------------------------------

# 18. Când folosesc ce?

  Situație                   Comandă
  -------------------------- ----------------------
  vezi modificări            git status
  salvezi                    git add + git commit
  trimiți                    git push
  iei ultimele modificări    git pull
  funcționalitate nouă       git switch -c
  unești branch-uri          git merge
  istoric liniar             git rebase
  pauză temporară            git stash
  ai stricat ceva            git reflog
  revii la o stare           git reset
  anulezi un commit public   git revert

------------------------------------------------------------------------

# 19. Reguli de aur

-   Fă commit-uri mici și descriptive.
-   Nu lucra direct pe `main`.
-   Folosește branch-uri pentru fiecare funcționalitate.
-   Trage (`git pull`) înainte să împingi (`git push`).
-   Înainte de `reset --hard`, verifică de două ori.
-   Dacă ai făcut o prostie, începe cu `git reflog`.

------------------------------------------------------------------------

# 20. Cele mai folosite comenzi

``` bash
git status
git add .
git commit -m "Mesaj"
git push
git pull
git switch
git switch -c
git branch
git merge
git rebase
git stash
git reflog
git reset
git revert
git log --oneline --graph --decorate --all
```
