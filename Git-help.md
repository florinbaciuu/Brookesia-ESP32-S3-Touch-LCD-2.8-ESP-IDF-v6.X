# Git Quick Help

Un ghid scurt cu comenzile pe care le vei folosi cel mai des.

------------------------------------------------------------------------

## Verificare

### Vezi starea repository-ului

``` bash
git status
```

### Vezi branch-ul curent

``` bash
git branch --show-current
```

### Vezi toate branch-urile locale

``` bash
git branch
```

### Vezi branch-urile remote

``` bash
git branch -r
```

### Vezi toate branch-urile

``` bash
git branch -a
```

------------------------------------------------------------------------

## Branch-uri

### Creează un branch nou și comută pe el

``` bash
git switch -c nume-branch
```

Exemplu:

``` bash
git switch -c feature/audio
```

### Comută pe un branch existent

``` bash
git switch main
```

### Șterge un branch local

``` bash
git branch -d nume-branch
```

### Șterge forțat un branch local

``` bash
git branch -D nume-branch
```

### Șterge un branch de pe GitHub

``` bash
git push origin --delete nume-branch
```

------------------------------------------------------------------------

## Salvarea modificărilor

### Adaugă toate fișierele

``` bash
git add .
```

### Creează un commit

``` bash
git commit -m "Mesajul commit-ului"
```

### Trimite modificările pe GitHub

``` bash
git push
```

### Prima trimitere pentru un branch nou

``` bash
git push -u origin nume-branch
```

### Adu modificările de pe GitHub

``` bash
git pull
```

------------------------------------------------------------------------

## Istoric

### Vezi istoricul commit-urilor

``` bash
git log
```

### Istoric scurt

``` bash
git log --oneline --graph --decorate --all
```

------------------------------------------------------------------------

## Verificare remote

### Vezi repository-ul remote

``` bash
git remote -v
```

### Vezi branch-urile existente pe GitHub

``` bash
git ls-remote --heads origin
```

### Curăță referințele remote vechi

``` bash
git fetch --prune
```

------------------------------------------------------------------------

# Workflow recomandat

## Lucrezi la o funcționalitate nouă

``` bash
git switch -c feature/nume
git add .
git commit -m "Descriere"
git push -u origin feature/nume
```

## Revii pe main

``` bash
git switch main
git pull
```

## Ștergi branch-ul după merge

``` bash
git branch -D feature/nume
git push origin --delete feature/nume
```

------------------------------------------------------------------------

# Comenzile pe care merită să le înveți primele

-   git status
-   git add
-   git commit
-   git push
-   git pull
-   git switch
-   git branch
-   git log
