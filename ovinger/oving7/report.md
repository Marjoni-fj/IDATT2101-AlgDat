# Øving 7: Korteste vei med Dijkstra og ALT

**Emne:** IDATT2101 Algoritmer og datastrukturer
**Navn:** **[FYLL INN]**

---

## 1. Oppgaven

Programmet skal finne raskeste vei på veinettet i Norden (OpenStreetMap-data, kjøretid som kantvekt) med to algoritmer: Dijkstras algoritme og ALT (A\* med landemerker og trekantulikheten). Programmet skal også finne de 4 nærmeste interessepunktene av en gitt type, og vise reiseruter og interessepunkter på kart. Begge algoritmene måles på tidsbruk og antall noder som tas ut av prioritetskøen.

Programmet er skrevet i C (C17) og kompilert med `gcc -O2`. Algoritmene, heapen og preprosesseringen er implementert selv, uten ferdige rutingbibliotek.

## 2. Implementasjon

### 2.1 Grafen

Nodene ligger i en tabell indeksert på nodenummer, og hver node har en lenket liste med utgående kanter (`Edge`: destinasjon, kjøretid i hundredels sekunder). Kartet har 7 956 886 noder og 17 815 613 kanter. Samtidig som kantene leses inn bygges også den **omvendte grafen** (alle kanter snudd). Den trengs for å regne ut avstand *til* landemerkene (se 2.5), fordi veinettet har enveiskjørte veier og rundkjøringer.

Kjøretiden brukes som kantvekt, og lagres som `int` (hundredels sekunder), slik oppgaven foreslår.

### 2.2 Prioritetskø

Prioritetskøen er en binær min-heap i en tabell (`HeapElement`: nodenummer og prioritet). Når en node får en kortere avstand, legges den inn i heapen på nytt med ny prioritet, og den gamle oppføringen blir liggende. Hver node har et `processed`-flagg: når en oppføring tas ut av heapen og noden allerede er behandlet, hoppes oppføringen over. Dette gir riktig rekkefølge, fordi den nye oppføringen alltid har lavere prioritet og dermed kommer ut før den gamle. Heapen kan derfor inneholde flere oppføringer enn det finnes noder, og den utvides ved behov med `realloc`.

Fordelen er at vi slipper å holde styr på hvor hver node ligger i heapen. Prisen er litt større heap.

### 2.3 Dijkstra og ALT

Dijkstra og ALT bruker samme søkefunksjon. Prioriteten til en node er

```
prioritet = avstand fra start + estimat for avstand til mål
```

Dijkstra er det spesialtilfellet der estimatet er 0. Søket stopper når **målnoden tas ut av køen**, ikke når den første gang oppdages, siden den første veien til målet ikke nødvendigvis er den raskeste. Kjøretiden for ruta leses direkte fra målnodens avstandsfelt, og beregnes altså ikke på nytt ved å summere kanter langs ruta. Ruta bygges ved å følge `previous`-pekerne fra målet tilbake til start.

### 2.4 ALT-estimatet

For hvert landemerke L gir trekantulikheten to nedre grenser for avstanden fra en node *n* til målet *t*:

- `d(L, t) − d(L, n)`
- `d(n, L) − d(t, L)`

Estimatet er den største av grensene over alle landemerkene, og minst 0. Tallene hentes fra to tabeller (avstand *fra* og *til* hvert landemerke). Hvis en avstand er `INT_MAX` (noden kan ikke nås), hoppes den grensen over. Siden hver grense er en gyldig nedre grense, og maksimum av slike også er det, forkaster ikke ALT den korteste veien.

### 2.5 Preprosessering og lagring

For hvert landemerke kjøres Dijkstra til køen er tom, først på den vanlige grafen (avstand fra landemerket) og så på den omvendte grafen (avstand til landemerket). Resultatet er to tabeller på 4 × 7 956 886 `int` (til sammen ca. 255 MB). Tabellene lagres i en binærfil, `landmarks.bin`, med antall noder og landemerke-id-ene først, slik at preprosesseringen bare trengs første gang. Ved senere kjøringer leses filen inn, og programmet sjekker at antall noder stemmer med kartet.

Preprosesseringen tok **[FYLL INN: tid, f.eks. «ca. X minutter». Mål ved å slette `landmarks.bin` og kjøre med `time`. Fjern linjen hvis du ikke har målt]**.

### 2.6 Interessepunkter

Interessepunktene (`interessepkt.txt`) leses inn og sorteres på nodenummer. Nærmeste interessepunkter finnes med Dijkstra fra startnoden, som stopper så snart 4 treff av riktig type er funnet. For hver node som tas ut av køen slås eventuelle interessepunkter opp med binærsøk. Typen testes med bitvis OG mot typekoden: `8 | 16` for spise- eller drikkested, og `4` for ladestasjon. Dermed treffer også steder som har flere typer.

### 2.7 Tidtaking og telling

Tiden måles med `clock_gettime(CLOCK_MONOTONIC)` rundt selve søket. Innlesing av kartfilene, preprosessering, bygging av ruta og utskrift er **ikke** med. Tidtakingen inkluderer derimot nullstillingen av søkedataene (avstand, forgjenger og flagg for alle ca. 8 millioner noder) og opprettelsen av heapen, siden dette er en del av søkefunksjonen. Det er en fast kostnad som er lik for Dijkstra og ALT.

«Prosesserte noder» teller hver node første gang den tas ut av køen. Gamle, utdaterte oppføringer som hoppes over, telles ikke.

### 2.8 Visualisering

Programmet skriver hver rute og hvert sett med interessepunkter til en GeoJSON-fil (koordinater i rekkefølgen lengdegrad, breddegrad, slik GeoJSON krever). Filene er vist på kart med geojson.io.

## 3. Landemerker

Programmet bruker 4 landemerker:

| Landemerke | Nodenummer |
|---|---|
| Oslo | 2948202 |
| Trondheim | 7826348 |
| København | 545911 |
| Helsinki | 925129 |

**Hvorfor disse:** Estimatet blir best når landemerke, start og mål ligger omtrent på linje (landemerke–start–mål eller start–mål–landemerke), og derfor bør landemerkene ligge rundt kanten av kartet. Landemerkene er valgt for å dekke hver sin kant av kartet: Trondheim midt i Norge, Oslo i sørøst, København i sør og Helsinki i øst. For de to påkrevde rutene ligger Trondheim like ved startpunktet (Gløshaugen ligger i Trondheim), og København ligger like forbi målet (Odense), så ruta Gløshaugen–Odense går omtrent på linjen Trondheim–start–mål–København. Oslo og Helsinki skal gi gode estimater for lange turer gjennom Sør-Norge, Sverige og Finland. Fire landemerker ligger innenfor de 3–5 oppgaven anbefaler, og gir en rimelig balanse mellom estimatkvalitet og minnebruk (hvert landemerke koster ca. 64 MB).

**[TILPASS: bytt ut eller utvid med dine egne grunner hvis de var andre.]**

Det ligger ingen landemerker i Nord-Norge eller nord i Finland. Vi har ikke testet om et landemerke der ville gitt færre noder på nordlige turer.

## 4. Resultater

Maskin: **[FYLL INN: CPU og RAM]**, Linux, `gcc -O2 -std=c17`. Hele programmet, inkludert innlesing av kartet og landemerkefilen, tok ca. 6,1 s.

### 4.1 Gløshaugen–Otilienborg og Gløshaugen–Odense

Nodenumre: Gløshaugen 2001238, Otilienborg 1987066, Odense 2763740.

| Rute | Algoritme | Reisetid (tt:mm:ss) | Søketid | Noder fra køen | Noder i ruta |
|---|---|---|---|---|---|
| Gløshaugen–Otilienborg | Dijkstra | 00:03:56 | 0,0062 s | 3 565 | 53 |
| Gløshaugen–Otilienborg | ALT | 00:03:56 | 0,0075 s | 581 | 53 |
| Gløshaugen–Odense | Dijkstra | 12:46:41 | 0,818 s | 5 053 533 | 3 691 |
| Gløshaugen–Odense | ALT | 12:46:41 | 0,229 s | 1 624 980 | 3 691 |

Tidene varierte litt mellom kjøringene. På Odense-ruta lå Dijkstra mellom 0,81 og 0,86 s og ALT mellom 0,23 og 0,24 s over flere kjøringer. Antall noder var identisk hver gang.

![Skjermdump av programutskriften](bilder/utskrift.png)

*Skjermdump av programutskriften med reisetid, søketid og antall noder for begge algoritmer.*

### 4.2 Reiseruter på kart

![Gløshaugen–Otilienborg](bilder/rute-otilienborg.png)

*Gløshaugen–Otilienborg, 00:03:56.*

![Gløshaugen–Odense](bilder/rute-odense.png)

*Gløshaugen–Odense, 12:46:41.*

Dijkstra og ALT ga samme kjøretid og samme antall noder i ruta på begge turene. Kartene viser ALT-ruta. **[BEKREFT: kjør `diff dijkstra-gloshaugen-odense.geojson alt-gloshaugen-odense.geojson`. Hvis tom utskrift, kan du skrive «rutene er identiske»]**

### 4.3 Nærmeste interessepunkter

Avstandene er kjøretid fra startnoden.

**4 nærmeste spise- eller drikkesteder fra Gløshaugen (node 2001238):**

| # | Navn | Node | Kjøretid |
|---|---|---|---|
| 1 | SiT Kafe Kjelhuset | 1941801 | 33,6 s |
| 2 | Kantine Materialteknologi 2 | 2583327 | 47,1 s |
| 3 | Kromaten kantine | 6990844 | 52,0 s |
| 4 | Cafe-sito Stripa | 1957896 | 54,9 s |

![De 4 nærmeste spise- og drikkestedene ved Gløshaugen](bilder/poi-gloshaugen.png)

**4 nærmeste ladestasjoner fra Odense (node 2763740):**

| # | Navn | Node | Kjøretid |
|---|---|---|---|
| 1 | Clever Føtex Bolbro | 5926145 | 3 min 37 s |
| 2 | Tesla | 3257221 | 5 min 48 s |
| 3 | Tesla Supercharger Odense | 355152 | 6 min 12 s |
| 4 | Clever | 1319864 | 6 min 37 s |

![De 4 nærmeste ladestasjonene ved Odense](bilder/poi-odense.png)

### 4.4 Kontroll mot fasit

For å sjekke at begge algoritmene er riktige ble de kjørt på flere turer fra fasiten (`lf7.pdf`). Alle ga nøyaktig riktig kjøretid, både med Dijkstra og ALT.

| Tur | Fasit (tid) | Dijkstra: noder fra køen / tid | ALT: noder fra køen / tid | Noder i ruta |
|---|---|---|---|---|
| Kårvåg–Gjemnes | 00:40:47 | 13 406 / 0,0089 s | 4 295 / 0,0064 s | 334 |
| Jyväskylä–Kárášjohka | 11:03:16 | 2 607 364 / 0,432 s | 291 807 / 0,057 s | 3 451 |
| Gol–Lakselv Hotell | 21:07:07 | 7 801 017 / 1,257 s | 1 199 115 / 0,178 s | 7 518 |
| Esso Sauda–Tervo | 21:44:14 | 7 442 387 / 1,192 s | 2 170 927 / 0,349 s | 5 796 (Dijkstra), 5 791 (ALT) |
| Nørrebro–Malmö Brygghus | 00:33:35 | 203 117 / 0,042 s | 21 544 / 0,013 s | 410 |

På Esso Sauda–Tervo fant ALT en rute med 5 791 noder mot 5 796 for Dijkstra, men med nøyaktig samme kjøretid. Det er tillatt: to ulike ruter kan ha lik kjøretid, og begge er da raskeste vei.

## 5. Diskusjon

**Antall noder.** ALT tok ut færre noder fra køen enn Dijkstra på alle turene, fra 3,1 ganger færre (Gløshaugen–Odense og Kårvåg–Gjemnes) til 9,4 ganger færre (Nørrebro–Malmö). Dijkstra sprer seg som en sirkel rundt startnoden. ALT bruker landemerkene til å lete mer i retning mot målet.

**Tid.** På de fleste rutene er ALT også raskere i tid, fra 3,2 til 7,6 ganger. Mest gevinst ga Jyväskylä–Kárášjohka (8,9 ganger færre noder, 7,6 ganger raskere). Gløshaugen–Odense er 3,6 ganger raskere.

På Gløshaugen–Otilienborg er ALT derimot litt *tregere* (0,0075 s mot 0,0062 s), selv om den tar ut 6 ganger færre noder, og på Kårvåg–Gjemnes er forskjellen liten (1,4 ganger). Vi har ikke målt årsaken, men to forklaringer er sannsynlige:

1. Hvert søk starter med å nullstille søkedata for alle ca. 8 millioner noder og opprette heapen. Denne faste kostnaden er med i tiden, og er stor sammenlignet med selve søket når søket bare berører noen tusen noder.
2. ALT må slå opp i tabeller på 255 MB for hver node som legges i køen. Oppslagene treffer ulike steder i minnet, og gir mange cache-bom. Oppgaven nevner selv at ALT bruker mer minne, og at det påvirker tiden.

**Kravet om under 15 sekunder.** Den tregeste Dijkstra-kjøringen (Gol–Lakselv Hotell, der nesten hele kartet behandles: 7,8 millioner av 7,96 millioner noder) tok 1,26 s, altså langt under grensen. Innlesing av kartet og preprosessering er ikke med i disse tidene.

**Mulige forbedringer.**

- Estimatet regnes nå ut på nytt hver gang en node får kortere avstand. Siden estimatet til en node ikke endrer seg, kunne det vært lagret i noden og regnet ut én gang.
- Nullstillingen av søkedata kunne vært begrenset til nodene det forrige søket faktisk berørte, slik at korte søk ikke betaler for hele kartet.
- Flere landemerker, eller et landemerke lenger nord, kunne gitt færre noder på enkelte ruter, til en pris av mer minne.

## 6. Kjøring og innlevering

Programmet kompileres med

```
gcc -O2 -Wall -Wextra -Wpedantic -std=c17 oving7.c -o oving7
```

og forventer `noder.txt`, `kanter.txt` og `interessepkt.txt` i samme mappe. Første kjøring lager `landmarks.bin`. Kartfilene og `landmarks.bin` er ikke lagt ved innleveringen, siden kartfilene allerede er utlagt av faglærer. Kompilering uten `-O2` ga omtrent dobbelt så lange søketider (Dijkstra Gløshaugen–Odense 1,69 s uten og ca. 0,82 s med `-O2`).