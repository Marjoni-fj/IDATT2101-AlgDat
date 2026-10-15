# Øving 5: Edmonds-Karp, maksimal flyt

**Navn:** Fredrik Jonathan Marjoni
**Emne:** IDATT2101 Algoritmer og datastrukturer

## 1. Problemet

Gitt en rettet graf der hver kant har en kapasitet, en kilde og et sluk. Oppgaven er å finne den største flyten som kan sendes fra kilden til sluket.
En gyldig flyt må oppfylle to regler:

1. Flyten på en kant er aldri større enn kapasiteten.
2. For alle noder unntatt kilden og sluket er flyt inn lik flyt ut.

## 2. Hvordan algoritmen virker

Edmonds-Karp er Ford-Fulkerson der den flytøkende veien alltid er den **korteste** (færrest kanter), funnet med bredde-først-søk (BFS).

- **Restkapasitet** for en kant er `kapasitet - flyt`.
- **Restnettet** består av alle kanter med restkapasitet større enn 0. BFS søker bare i restnettet.
- **Motsatte kanter:** hver kant har en motsatt kant med kapasitet 0. Når det sendes flyt langs en kant, trekkes den samme flyten fra den motsatte kanten. Flyten der blir negativ, og dermed får den motsatte kanten restkapasitet. Det lar algoritmen angre flyt den har sendt tidligere (for eksempel snur flyten på kanten 4→3 i flytgraf1 til slutt).

Algoritmen gjentar disse stegene:

1. Kjør BFS fra kilden i restnettet.
2. Hvis sluket ikke nås, stopp. Flyten er maksimal.
3. Gå bakover fra sluket via forgjengerne og finn kanten med lavest restkapasitet (flaskehalsen).
4. Øk flyten med flaskehalsen på alle kanter langs veien, og trekk den fra de motsatte kantene.
5. Legg flaskehalsen til total flyt, og gå til steg 1.

## 3. Implementasjon (C)

- Grafen lagres som **nabolister**. `first[u]` peker på en lenket liste med alle kanter som starter i node `u`, koblet sammen med `next`.
- Hver kant (`Edge`) har feltene `target`, `capacity`, `flow`, `next` og `reverse`.
- `addEdge` lager to kanter per linje i filen: den ekte (med kapasitet fra filen) og den motsatte (kapasitet 0). De peker på hverandre med `reverse`.
- BFS lagrer i `pred[v]` **kanten** som førte til noden `v` (ikke bare forrige node). Da kan flyten endres direkte på kantene langs veien uten å lete etter dem. Forrige node er `pred[v]->reverse->target`.
- Minnet frigjøres med `freeGraph`. Programmet ble kjørt med valgrind uten lekkasjer eller minnefeil.

## 4. Resultater

| Graf | Kilde | Sluk | Maksimal flyt | Antall flytøkende veier |
|------|-------|------|---------------|-------------------------|
| flytgraf1 | 0 | 7 | 10 | 8 |
| flytgraf2 | 0 | 1 | 27 | 4 |
| flytgraf3 | 0 | 1 | **42** | 7 |
| flytgraf4 | 0 | 7 | 11 | 9 |
| flytgraf5 | 0 | 7 | **90** | 7 |

For flytgraf1, 2 og 4 stemmer resultatene med fasiten i oppgaveteksten. For flytgraf1 er veiene identiske med eksempelet i oppgaven.

### flytgraf3 (kilde 0, sluk 1): maksimal flyt 42

```
Maksimum flyt fra 0 til 1 med Edmond-Karp
Økning : Flytøkende vei
    10  0 8 9 1 
     5  0 2 3 9 8 14 15 1 
    10  0 2 3 9 8 14 15 1 
     5  0 2 3 4 5 8 9 12 13 14 15 1 
     6  0 2 3 4 5 8 9 12 13 14 15 1 
     5  0 2 3 4 5 6 7 9 8 10 11 12 13 14 15 1 
     1  0 2 3 4 5 6 7 9 8 10 11 12 13 14 15 1 
Maksimal flyt for flytgraf3.txt ble 42
```

### flytgraf5 (kilde 0, sluk 7): maksimal flyt 90

```
Maksimum flyt fra 0 til 7 med Edmond-Karp
Økning : Flytøkende vei
    20  0 5 4 7 
    10  0 5 6 7 
     5  0 2 4 7 
    10  0 2 3 7 
    20  0 5 6 1 7 
     5  0 2 4 6 1 7 
    20  0 2 4 5 6 1 7 
Maksimal flyt for flytgraf5.txt ble 90
```

## 5. Kompleksitet

Fordi BFS alltid velger den korteste veien, kan veilengden aldri minke mellom to runder, og hver kant kan bare være flaskehals et begrenset antall ganger. Det gir maksimalt O(V·E) flytøkende veier. Hvert BFS koster O(E), så totalt blir tidsbruken **O(V·E²)**, uavhengig av kapasitetene.

## 6. Kompilering og kjøring

```
gcc -o oving5 oving5.c
./oving5
```

Programmet leser `flytgraf1.txt` til `flytgraf5.txt` fra samme mappe som det kjøres fra, og kjører alle fem grafene etter hverandre.
