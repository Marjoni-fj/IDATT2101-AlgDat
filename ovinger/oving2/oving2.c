#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*DISCLAIMER:
Denne koden er ikke samarbeidet med andre og alt innhold er kun skrevet av meg,
Fredrik Jonathan Marjoni. Noen av footnotene som er skrevet i koden var kun for
å hjelpe meg selv med å forstå koden og er ikke ment for å være en del av
innleveringen
*/

double method1(int n, double x) {
    if (n == 1) {
        return x;
    } else {
        return x * method1(n - 1, x);
    }
}

double method2(int n, double x) {
    /* Basistilfelle: n=1 i formelen fører til rekursivt kall med n=0
    (siden (n-1)/2 = 0 når n=1), så n==0 fungerer som det reelle
    basistilfellet her. x != 0 er med for å unngå at 0^0 gir 1.*/
    if (n == 0 && x != 0) {
        return 1;
    }
    if (n % 2 == 0) {
        return method2(n / 2, x * x);
    }
    return x * method2((n - 1) / 2, x * x);
}

double method3(int n, double x) { return pow(x, n); }

double method1(int number, double value);
double method2(int number, double value);
double method3(int number, double value);

typedef double (*FunctionPointer)(int, double);

void time_tracking(FunctionPointer methodx, int n, double x, int reps) {
    double result;
    clock_t start, end;

    start = clock();
    for (int i = 0; i < reps; i++) {
        result = methodx(n, x);
    }
    end = clock();

    double total_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    double time_per_call = total_time / reps;
    printf("n = %d, resultat = %f, tid per kall = %.9f sekunder\n", n, result,
           time_per_call);
}

int main() {
    // --- TEST: sjekk at metodene regner riktig, jf. 5^11 = 48828125 ---
    printf("=== TEST ===\n");
    printf("method1(11, 5.0) = %f (forventet 48828125)\n", method1(11, 5.0));
    printf("method2(11, 5.0) = %f (forventet 48828125)\n", method2(11, 5.0));
    printf("method3(11, 5.0) = %f (forventet 48828125)\n\n", method3(11, 5.0));

    /* x settes lavt for å unngå float-overflow ved store n (jfr. oppgavetipset)
     */
    const double x = 1.0025;
    const int reps = 100000;

    /*method1 er lineær rekursjon -> stack overflow-fare ved store n, holdes
    derfor lavere */
    int n_values_method1[] = {100, 500, 1000, 2000, 4000};
    int len1 = sizeof(n_values_method1) / sizeof(n_values_method1[0]);

    /*method2 og method3 tåler mye høyere n (method2 er logaritmisk i
    rekursjonsdybde)*/
    int n_values_method23[] = {100, 1000, 10000, 100000, 1000000};
    int len23 = sizeof(n_values_method23) / sizeof(n_values_method23[0]);

    printf("=== Tidtaking method1 (lineaer rekursjon) ===\n");
    for (int i = 0; i < len1; i++) {
        time_tracking(method1, n_values_method1[i], x, reps);
    }

    printf("\n=== Tidtaking method2 (halvering) ===\n");
    for (int i = 0; i < len23; i++) {
        time_tracking(method2, n_values_method23[i], x, reps);
    }

    printf("\n=== Tidtaking method3 (pow, kun til sammenligning) ===\n");
    for (int i = 0; i < len23; i++) {
        time_tracking(method3, n_values_method23[i], x, reps);
    }

    return 0;
}

/* Analyse av metodene gjennom asymptotisk kompleksitetsanalyse:
Eksempel på compilering av programmet:

=== TEST ===
method1(11, 5.0) = 48828125.000000 (forventet 48828125)
method2(11, 5.0) = 48828125.000000 (forventet 48828125)
method3(11, 5.0) = 48828125.000000 (forventet 48828125)

=== Tidtaking method1 (lineaer rekursjon) ===
n = 100, resultat = 1.283625, tid per kall = 0.000001467 sekunder
n = 500, resultat = 3.484903, tid per kall = 0.000007565 sekunder
n = 1000, resultat = 12.144546, tid per kall = 0.000015353 sekunder
n = 2000, resultat = 147.490003, tid per kall = 0.000030712 sekunder
n = 4000, resultat = 21753.300983, tid per kall = 0.000041281 sekunder

=== Tidtaking method2 (halvering) ===
n = 100, resultat = 1.283625, tid per kall = 0.000000030 sekunder
n = 1000, resultat = 12.144546, tid per kall = 0.000000056 sekunder
n = 10000, resultat = 69793169647.362289, tid per kall = 0.000000077 sekunder
n = 100000, resultat =
2742390033964132346774062061145037501934230843504014235541840803093368720583252405781382838708932093566517248.000000,
tid per kall = 0.000000106 sekunder n = 1000000, resultat = inf, tid per kall =
0.000000134 sekunder

=== Tidtaking method3 (pow, kun til sammenligning) ===
n = 100, resultat = 1.283625, tid per kall = 0.000000014 sekunder
n = 1000, resultat = 12.144546, tid per kall = 0.000000014 sekunder
n = 10000, resultat = 69793169647.356567, tid per kall = 0.000000014 sekunder
n = 100000, resultat =
2742390033961885805721633525833768519143659937617854945726823112631227154360962825073701442349191146137190400.000000,
tid per kall = 0.000000014 sekunder n = 1000000, resultat = inf, tid per kall =
0.000000011 sekunder
*/

/*
Det første vi ser er at algoritmene kommer frem til samme svar som matte
bibliotekets pow-funksjon når vi gir dem samme input av 5^11, noe som indikerer
at algoritmene er implementert riktig.

Den første algoritmen (method1) er en lineær rekursiv algoritme som har
tidskompleksitet T(n) = T(n - 1) + c. Dersom vi skal løse denne rekursjonen får
man at antall rekursive kall er proporsjonalt med n. Det gir kjøretid Θ(n),
altså lineær. Dette betyr at tiden det tar å beregne x^n vokser lineært med n.
Dette kan sees i tidtakingsresultatene, hvor tiden per kall øker med økende n.

n = 100, resultat = 1.283625, tid per kall = 0.000001467 sekunder
n = 1000, resultat = 12.144546, tid per kall = 0.000015353 sekunder


Merk at mestermetoden ikke kan brukes direkte på metode 1, fordi den rekursive
relasjonen T(n) = T(n-1) + c ikke er på formen a·T(n/b); problemet krymper med
en konstant (n-1), ikke ved divisjon (n/b). Denne typen rekurrens løses i stedet
med substitusjon/utrulling:

T(n) = T(n-1) + c = T(n-2) + 2c = ... = T(1) + (n-1)c = Θ(n)
*/

/*
Den andre algoritmen (method2) tar for seg en halveringsteknikk, hvor vi
reduserer problemet til å beregne x^(n/2) og deretter kvadrere resultatet.
method2 halverer n i hvert rekursive kall (enten n/2 direkte, eller (n-1)/2 ved
oddetall). Ettersom enhver sammenligning, modulo-regning og multiplikasjon tar
konstant tid, er tidskompleksitetsfunksjonen for method2: T(n) = T(n/2) + c

For å finne tidskompleksiteten i Θ-notasjon, kan vi bruke mestermetoden.

T(n) = { T(1) = c,          n = 1
        aT(n/2) + c*n^0,    n > 1 }


hvor vi har følgende sammenhenger:
a < b^k -> Θ(n^k)
a = b^k -> Θ(n^k * log(n))
a > b^k -> Θ(n^(log_b(a)))

Her er a = 1, b = 2, k = 0. Setter vi dette inn i mestermetoden får vi at
1 vs 2^0, altså 1 = 1, som betyr at a = b^k. Dermed får vi tilfelle at T(n) =
Θ(n^k * log(n)) Fullfører vi uttrykket for Θ får vi T(n) = Θ(n^0 * log(n)) =
Θ(log(n)). Dette betyr at tiden det tar å beregne x^n vokser logaritmisk med n.
Dette kan sees i tidtakingsresultatene, hvor tiden per kall øker svært lite med
økende n.

For eksempel tidobler vi n, vil vi se at log(10n) = log(n) + log(10), som betyr
at tiden per kall øker med et konstant tillegg på log₂(10) (ca. 3.32), og ikke
proporsjonalt med n som i method1.


n = 1000, resultat = 12.144546, tid per kall = 0.000000056 sekunder
n = 10000, resultat = 69793169647.362289, tid per kall = 0.000000077 sekunder

*/

/*
Den tredje metoden (pow fra math biblioteket) har konstant kjøretid Θ(1)
uavhengig av n, siden den ikke bruker gjentatt multiplikasjon, men beregner x^n
via identiteten x^n = e^(n·ln x). Dette bekreftes av tidsmålingene, hvor tiden
per kall holder seg stabil rundt 14 ns uansett om n er 100 eller 1 000 000.

Ved n = 1 000 000 gir både method2 og method3 resultatet inf. Dette skyldes
flyttalls-overflow: med x = 1.0025 blir eksponenten n·ln(x) ≈ 2497, og en double
kan maksimalt representere verdier opp til ca. e^709. At overflowen skjer for
begge metodene samtidig, bekrefter at det er selve tallverdien x^n som
overflower, ikke en feil i implementasjonen.
*/