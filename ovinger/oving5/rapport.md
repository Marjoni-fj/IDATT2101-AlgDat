# Øving 5 Rapport: Edmonds-Karp, maksimal flyt

**Navn:** Fredrik Jonathan Marjoni
**Emne:** IDATT2101 Algoritmer og datastrukturer

## 1. Problemet

Vi har en rettet graf der hver kant har en kapasitet, og to spesielle noder: en kilde og et sluk. Kapasiteten sier hvor mye som maks kan sendes gjennom kanten. Oppgaven er å finne den største mengden flyt som kan sendes fra kilden til sluket.

Flyten må følge to regler: ingen kant kan få mer flyt enn kapasiteten, og for alle noder mellom kilden og sluket må det gå like mye flyt inn som ut.

## 2. Hvordan algoritmen virker

Edmonds-Karp er en variant av Ford-Fulkerson. Ford-Fulkerson sier at man gjentatte ganger skal finne en vei fra kilden til sluket med ledig plass, og sende flyt langs den. Den sier ikke hvordan veien skal velges. Edmonds-Karp bestemmer at man alltid skal ta den **korteste** veien (færrest kanter), og det gjør man med bredde-først-søk (BFS).

**Restkapasitet** for en kant er `kapasitet - flyt`. **Restnettet** er alle kanter som har restkapasitet større enn 0, og det er i dette nettet BFS søker.

**Motsatte kanter:** Hver kant har en motsatt kant med kapasitet 0. Eksempel fra flytgraf1: kanten 0→4 har kapasitet 3. Hvis vi sender 2 langs den, får 0→4 flyt 2 og restkapasitet 1. Den motsatte kanten 4→0 har kapasitet 0, men får flyt -2, så restkapasiteten blir 0 - (-2) = 2. Det betyr at vi senere kan sende 2 "tilbake", altså angre flyt vi har sendt. Slik kan algoritmen ombestemme seg. I flytgraf1 snur for eksempel flyten på kanten 4→3 til slutt.

Algoritmen gjør dette i en løkke:

1. Kjør BFS fra kilden til sluket, og følg bare kanter med restkapasitet større enn 0. Hver node husker hvilken kant den ble nådd via.
2. Finner BFS ingen vei til sluket, er vi ferdige, og flyten er maksimal.
3. Gå bakover fra sluket til kilden og finn kanten med minst restkapasitet. Det er flaskehalsen, og den bestemmer hvor mye vi kan sende.
4. Gå bakover en gang til. Øk flyten med flaskehalsen på hver kant, og trekk den samme mengden fra den motsatte kanten.
5. Legg flaskehalsen til total flyt, og start på nytt med et nytt BFS. Restnettet har nå endret seg, så BFS kan finne en annen vei.

## 3. Implementasjon (C)

- Grafen lagres med nabolister. `first` er en tabell der indeksen er en node, og verdien er starten på en lenket liste med alle kanter som går ut fra den noden. `next` brukes til å gå videre i listen.
- Hver kant (`Edge`) har `target`, `capacity`, `flow`, `next` og `reverse`. `reverse` peker på den motsatte kanten.
- `addEdge` lager to kanter for hver linje i filen: den ekte kanten, og den motsatte med kapasitet 0. De peker på hverandre via `reverse`.
- `bfs` fyller tabellen `pred`. For hver node husker `pred` hvilken **kant** BFS kom langs for å nå noden. Går man bakover fra sluket via `pred`, kommer man til kilden, og da har man den flytøkende veien.
- Jeg lagrer kanten og ikke bare forrige node. Da kan jeg endre `flow` direkte på kantene langs veien uten å lete etter dem, og jeg får forrige node via `pred[v]->reverse->target`.
- `freeGraph` frigjør alle kantene mellom hver graf. Programmet ble kjørt med `valgrind` uten lekkasjer eller minnefeil.

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

Tidsbruken er O(V·E²). Hver BFS besøker hver kant maks én gang og koster derfor O(E). Siden BFS alltid velger den korteste veien, kan veilengden aldri bli kortere mellom to runder, og hver kant kan bare være flaskehals et begrenset antall ganger. Det gir maksimalt O(V·E) flytøkende veier, og totalt O(V·E) × O(E) = O(V·E²). Dette gjelder uansett hva kapasitetene er.

## 6. Kompilering og kjøring

```
gcc -Wall -Wextra -g -o oving5 oving5.c
./oving5
```

Programmet leser `flytgraf1.txt` til `flytgraf5.txt` fra samme mappe som det kjøres fra, og kjører alle fem grafene etter hverandre.