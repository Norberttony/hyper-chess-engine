
#include "test-suite.h"


pos positions[NUM_TEST_SUITE_POSITIONS] =
{
    {
		"Starting position",
		"unbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNU w 0 1",
		{ 1ULL, 32ULL, 944ULL, 42738ULL, 1846916ULL },
		{ 0ULL, 0ULL, 0ULL, 162ULL, 11143ULL },
		{ 0ULL, 0ULL, 0ULL, 162ULL, 11143ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Chameleon-coordinator checkmate",
		"k5R1/p7/8/8/7K/8/8/5B2 w 0 -",
		{ 1ULL, 46ULL, 605ULL, 27058ULL, 373502ULL },
		{ 0ULL, 0ULL, 1ULL, 132ULL, 1197ULL },
		{ 0ULL, 0ULL, 1ULL, 132ULL, 1197ULL },
		{ 0ULL, 3ULL, 0ULL, 241ULL, 0ULL }
	},
	{
		"White king-chameleon death squares with king move",
		"unbqkbn1/pppppppp/3PPP2/3r1B2/8/8/PPP3PP/RNBQK1NU w 0 8",
		{ 1ULL, 63ULL, 2067ULL, 126668ULL, 4757621ULL },
		{ 0ULL, 1ULL, 82ULL, 2466ULL, 204926ULL },
		{ 0ULL, 1ULL, 82ULL, 2466ULL, 204927ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 237ULL }
	},
	{
		"Black king-chameleon death squares with king move",
		"un1q1kb1/pp1ppppn/P2PPP1p/4RB1b/8/2p4P/1PPK2P1/1NBQ2NU b 0 16",
		{ 1ULL, 41ULL, 2889ULL, 119455ULL },
		{ 0ULL, 2ULL, 50ULL, 6093ULL },
		{ 0ULL, 2ULL, 50ULL, 6094ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"straddler custodial capture only after move made",
		"p1P4k/4p3/Pp2P2p/3Pp3/p1Pp4/1Pp2P2/P2u4/3Pp2K w 0 -",
		{ 1ULL, 47ULL, 2850ULL, 133921ULL },
		{ 0ULL, 8ULL, 215ULL, 18819ULL },
		{ 0ULL, 10ULL, 217ULL, 22264ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"straddler and chameleon duo",
		"p1PP1P1k/4pp2/Pp2P2p/3Pp3/p1Pq1P2/1Bp2B2/P2p4/3Pp1pK w 0 -",
		{ 1ULL, 56ULL, 3554ULL, 207069ULL },
		{ 0ULL, 13ULL, 227ULL, 40786ULL },
		{ 0ULL, 15ULL, 227ULL, 45588ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"straddlers only work with other friendly straddlers",
		"p1P4k/4p2N/Ppp1P2p/3Pp3/p1Pp4/1Pp2P1P/P2u4/K2Pp2N w 0 -",
		{ 1ULL, 58ULL, 3251ULL, 189823ULL },
		{ 0ULL, 7ULL, 395ULL, 21045ULL },
		{ 0ULL, 9ULL, 444ULL, 25746ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"straddler-chameleon duo and co (cannot work with them though)",
		"p1PP1P1k/4pp1N/Pp2P2p/3Pp3/p1Pq1P2/1Bp2b1B/P2p4/3Pp1pK w 0 -",
		{ 1ULL, 61ULL, 4277ULL, 261315ULL },
		{ 0ULL, 12ULL, 411ULL, 46888ULL },
		{ 0ULL, 13ULL, 411ULL, 50578ULL },
		{ 0ULL, 0ULL, 56ULL, 6ULL }
	},
	{
		"springer test 1",
		"4p2k/1b2p3/6p1/5p2/1p1pN2p/8/2n1P3/K6n w 0 -",
		{ 1ULL, 20ULL, 2121ULL, 53013ULL },
		{ 0ULL, 3ULL, 61ULL, 4977ULL },
		{ 0ULL, 3ULL, 61ULL, 4977ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"chameleon-springer test",
		"4p2k/1b2p3/6p1/5p2/1p1pB2p/8/2n1P3/K6n w 0 -",
		{ 1ULL, 18ULL, 1905ULL, 43518ULL },
		{ 0ULL, 1ULL, 47ULL, 891ULL },
		{ 0ULL, 1ULL, 47ULL, 891ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"king and coordinator basics",
		"Pk5p/r7/8/8/p4R2/8/8/K4n1p w 0 -",
		{ 1ULL, 23ULL, 1258ULL, 30237ULL },
		{ 0ULL, 16ULL, 88ULL, 7121ULL },
		{ 0ULL, 18ULL, 89ULL, 7801ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"king-chameleon duo",
		"Pk5p/r6B/8/8/p4B2/8/8/K4n1p w 0 -",
		{ 1ULL, 41ULL, 2064ULL, 81600ULL },
		{ 0ULL, 9ULL, 189ULL, 10069ULL },
		{ 0ULL, 9ULL, 189ULL, 10089ULL },
		{ 0ULL, 0ULL, 0ULL, 1ULL }
	},
	{
		"coordinator-chameleon duo",
		"R5k1/8/8/B7/p3B3/8/8/K4n1P w 0 -",
		{ 1ULL, 60ULL, 1425ULL, 91294ULL },
		{ 0ULL, 1ULL, 16ULL, 2377ULL },
		{ 0ULL, 1ULL, 16ULL, 2415ULL },
		{ 0ULL, 1ULL, 0ULL, 845ULL }
	},
	{
		"immobilizer checkmates",
		"N4B1B/8/4k3/3u1U2/4K3/8/8/8 w 0 -",
		{ 1ULL, 55ULL, 778ULL, 48371ULL },
		{ 0ULL, 0ULL, 0ULL, 164ULL },
		{ 0ULL, 0ULL, 0ULL, 164ULL },
		{ 0ULL, 1ULL, 0ULL, 1170ULL }
	},
	{
		"immobilized immobilizer",
		"N4B1B/8/4k3/4uU2/4K3/8/8/8 w 0 -",
		{ 1ULL, 38ULL, 0ULL },
		{ 0ULL, 0ULL, 0ULL },
		{ 0ULL, 0ULL, 0ULL },
		{ 0ULL, 5ULL, 0ULL }
	},
	{
		"chameleoned immobilizer",
		"N4B1B/8/4k3/4bU2/4K3/8/8/8 w 0 -",
		{ 1ULL, 45ULL, 0ULL },
		{ 0ULL, 1ULL, 0ULL },
		{ 0ULL, 1ULL, 0ULL },
		{ 0ULL, 11ULL, 0ULL }
	},
	{
		"retractor",
		"qP5k/PP6/8/3p1P2/3pQp2/3p4/8/7K w 0 -",
		{ 1ULL, 41ULL, 1187ULL, 57057ULL },
		{ 0ULL, 1ULL, 5ULL, 1334ULL },
		{ 0ULL, 1ULL, 5ULL, 1334ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"chameleon retractor",
		"pp5k/pp6/8/p1qp1QP1/PB1pBp2/3p4/8/7K w 0 -",
		{ 1ULL, 44ULL, 2183ULL, 107986ULL },
		{ 0ULL, 4ULL, 81ULL, 6390ULL },
		{ 0ULL, 4ULL, 81ULL, 6392ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"one move. five pieces. all captured.",
		"k3r2K/8/8/4P3/4p3/2Pp1pP1/4B3/4q3 w 0 -",
		{ 1ULL, 6ULL, 246ULL, 8750ULL, 434892ULL },
		{ 0ULL, 2ULL, 0ULL, 699ULL, 3124ULL },
		{ 0ULL, 6ULL, 0ULL, 1223ULL, 3124ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 5ULL }
	},
	{
		"chameleon pin",
		"pnp3k1/1B6/8/8/8/8/6R1/7K b 0 -",
		{ 1ULL, 2ULL, 62ULL, 1885ULL, 60568ULL },
		{ 0ULL, 1ULL, 0ULL, 40ULL, 581ULL },
		{ 0ULL, 1ULL, 0ULL, 40ULL, 581ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 21ULL }
	},
	{
		"Springer pin 1",
		"8/8/N3pk2/8/8/8/8/7K b 0 -",
		{ 1ULL, 10ULL, 204ULL, 3063ULL, 73275ULL },
		{ 0ULL, 0ULL, 10ULL, 10ULL, 1172ULL },
		{ 0ULL, 0ULL, 10ULL, 10ULL, 1172ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Springer pin 2",
		"8/8/N3pkp1/8/8/8/8/7K b 0 -",
		{ 1ULL, 24ULL, 489ULL, 10765ULL, 254955ULL },
		{ 0ULL, 0ULL, 12ULL, 18ULL, 6275ULL },
		{ 0ULL, 0ULL, 12ULL, 18ULL, 6275ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Coordinator pin",
		"1p4k1/8/8/8/8/8/8/1R4K1 b 0 -",
		{ 1ULL, 10ULL, 208ULL, 2099ULL, 51718ULL },
		{ 0ULL, 0ULL, 3ULL, 9ULL, 404ULL },
		{ 0ULL, 0ULL, 3ULL, 9ULL, 404ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 2ULL }
	},
	{
		"Retractor pin 1",
		"8/8/4b3/4Q3/4k3/8/8/7K b 0 -",
		{ 1ULL, 5ULL, 83ULL, 2133ULL, 47441ULL },
		{ 0ULL, 2ULL, 3ULL, 33ULL, 310ULL },
		{ 0ULL, 2ULL, 3ULL, 33ULL, 310ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pin 1",
		"k7/8/3pKBp1/8/8/8/8/8 w 0 -",
		{ 1ULL, 7ULL, 136ULL, 3362ULL, 83000ULL },
		{ 0ULL, 1ULL, 6ULL, 88ULL, 248ULL },
		{ 0ULL, 1ULL, 6ULL, 88ULL, 248ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pin 2",
		"k7/8/1pK2Bp1/8/8/8/8/8 w 0 -",
		{ 1ULL, 9ULL, 171ULL, 4564ULL, 110304ULL },
		{ 0ULL, 1ULL, 5ULL, 64ULL, 252ULL },
		{ 0ULL, 1ULL, 5ULL, 64ULL, 252ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pin 3",
		"k7/8/3pKB2/8/8/5p2/8/8 w 0 -",
		{ 1ULL, 9ULL, 201ULL, 4880ULL, 124893ULL },
		{ 0ULL, 1ULL, 0ULL, 100ULL, 94ULL },
		{ 0ULL, 1ULL, 0ULL, 100ULL, 94ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pin 4",
		"k4p2/8/3pKB2/8/8/5p2/8/8 w 0 -",
		{ 1ULL, 6ULL, 174ULL, 3497ULL, 116607ULL },
		{ 0ULL, 1ULL, 0ULL, 131ULL, 363ULL },
		{ 0ULL, 1ULL, 0ULL, 131ULL, 363ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pins springer 1",
		"k4p2/8/3pKN2/8/8/5p2/8/8 w 0 -",
		{ 1ULL, 6ULL, 174ULL, 3703ULL, 120280ULL },
		{ 0ULL, 1ULL, 0ULL, 337ULL, 366ULL },
		{ 0ULL, 1ULL, 0ULL, 337ULL, 366ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pins springer 2",
		"k7/8/3pKN2/8/8/5p2/8/8 w 0 -",
		{ 1ULL, 10ULL, 214ULL, 5518ULL, 132436ULL },
		{ 0ULL, 2ULL, 0ULL, 351ULL, 113ULL },
		{ 0ULL, 2ULL, 0ULL, 351ULL, 113ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"King pin 1",
		"ppp3k1/1K6/8/8/8/8/6R1/8 b 0 -",
		{ 1ULL, 2ULL, 58ULL, 851ULL, 20686ULL, 439048ULL },
		{ 0ULL, 0ULL, 8ULL, 4ULL, 2499ULL, 1688ULL },
		{ 0ULL, 0ULL, 8ULL, 4ULL, 2659ULL, 1688ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 40ULL, 0ULL }
	},
	{
		"Immobilizer chameleon pin",
		"pnp3k1/1B6/1u6/8/8/8/6R1/7K b 0 -",
		{ 1ULL, 25ULL, 515ULL, 16254ULL, 351379ULL },
		{ 0ULL, 0ULL, 10ULL, 154ULL, 6976ULL },
		{ 0ULL, 0ULL, 10ULL, 154ULL, 6993ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 92ULL }
	},
	{
		"Immobilizer springer pin 1",
		"8/8/N3pk2/u7/8/8/8/7K b 0 -",
		{ 1ULL, 35ULL, 370ULL, 11537ULL, 222510ULL },
		{ 0ULL, 0ULL, 3ULL, 0ULL, 4796ULL },
		{ 0ULL, 0ULL, 3ULL, 0ULL, 4796ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Immobilizer springer pin 2",
		"8/8/N3pkp1/u7/8/8/8/7K b 0 -",
		{ 1ULL, 42ULL, 391ULL, 15455ULL, 291256ULL },
		{ 0ULL, 0ULL, 5ULL, 0ULL, 7072ULL },
		{ 0ULL, 0ULL, 5ULL, 0ULL, 7072ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Immobilizer springer pin 3",
		"8/8/N5pk/u7/8/8/8/7K b 0 -",
		{ 1ULL, 34ULL, 399ULL, 14230ULL, 298358ULL },
		{ 0ULL, 0ULL, 5ULL, 0ULL, 5192ULL },
		{ 0ULL, 0ULL, 5ULL, 0ULL, 5192ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Immobilizer coordinator pin 1",
		"1p4k1/8/8/8/8/8/8/uR4K1 b 0 -",
		{ 1ULL, 30ULL, 366ULL, 6951ULL, 116938ULL },
		{ 0ULL, 0ULL, 8ULL, 12ULL, 1928ULL },
		{ 0ULL, 0ULL, 8ULL, 12ULL, 1928ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Immobilizer retractor pin 1",
		"8/8/4b3/4Qu2/4k3/8/8/7K b 0 -",
		{ 1ULL, 37ULL, 327ULL, 11452ULL, 204313ULL },
		{ 0ULL, 2ULL, 0ULL, 248ULL, 1148ULL },
		{ 0ULL, 2ULL, 0ULL, 248ULL, 1148ULL },
		{ 0ULL, 0ULL, 0ULL, 3ULL, 0ULL }
	},
	{
		"Immobilizer retractor pin 2",
		"8/8/4p3/4Qu2/4k3/8/8/7K b 0 -",
		{ 1ULL, 29ULL, 303ULL, 8339ULL, 158243ULL },
		{ 0ULL, 1ULL, 0ULL, 119ULL, 932ULL },
		{ 0ULL, 1ULL, 0ULL, 119ULL, 932ULL },
		{ 0ULL, 0ULL, 0ULL, 3ULL, 0ULL }
	},
	{
		"Immobilizer straddler pin 1",
		"k7/8/3pKBp1/4U3/8/8/8/8 w 0 -",
		{ 1ULL, 24ULL, 393ULL, 15711ULL, 320879ULL },
		{ 0ULL, 1ULL, 0ULL, 246ULL, 881ULL },
		{ 0ULL, 1ULL, 0ULL, 246ULL, 881ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Immobilizer straddler pin 2",
		"k7/8/1pK2Bp1/1U6/8/8/8/8 w 0 -",
		{ 1ULL, 25ULL, 333ULL, 14342ULL, 270825ULL },
		{ 0ULL, 1ULL, 0ULL, 182ULL, 580ULL },
		{ 0ULL, 1ULL, 0ULL, 182ULL, 580ULL },
		{ 0ULL, 0ULL, 0ULL, 2ULL, 0ULL }
	},
	{
		"Immobilizer straddler pin 3",
		"k7/8/3pKB2/4U3/8/5p2/8/8 w 0 -",
		{ 1ULL, 26ULL, 457ULL, 17528ULL, 355251ULL },
		{ 0ULL, 1ULL, 1ULL, 305ULL, 560ULL },
		{ 0ULL, 1ULL, 1ULL, 305ULL, 560ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Immobilizer straddler pin 4",
		"k4p2/8/3pKB2/4U3/8/5p2/8/8 w 0 -",
		{ 1ULL, 23ULL, 578ULL, 18518ULL, 511698ULL },
		{ 0ULL, 1ULL, 1ULL, 457ULL, 1916ULL },
		{ 0ULL, 1ULL, 1ULL, 457ULL, 1917ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Immobilizer straddler pin 5",
		"k7/5p2/3pKBU1/5p2/8/5p2/8/8 w 0 -",
		{ 1ULL, 26ULL, 673ULL, 17576ULL, 501595ULL },
		{ 0ULL, 3ULL, 2ULL, 1466ULL, 3825ULL },
		{ 0ULL, 3ULL, 2ULL, 1466ULL, 3843ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Immobilizer king pin",
		"ppp3k1/1K6/1u6/8/8/8/6R1/8 b 0 -",
		{ 1ULL, 23ULL, 416ULL, 14371ULL, 296308ULL },
		{ 0ULL, 0ULL, 38ULL, 24ULL, 39977ULL },
		{ 0ULL, 0ULL, 39ULL, 24ULL, 41783ULL },
		{ 0ULL, 0ULL, 0ULL, 42ULL, 10ULL }
	},
	{
		"Retractor-coordinator double pin",
		"6R1/8/4b3/4Q3/4k1p1/8/8/4K3 b 0 -",
		{ 1ULL, 6ULL, 212ULL, 3681ULL, 135246ULL },
		{ 0ULL, 0ULL, 7ULL, 100ULL, 4513ULL },
		{ 0ULL, 0ULL, 7ULL, 101ULL, 4558ULL },
		{ 0ULL, 0ULL, 1ULL, 2ULL, 150ULL }
	},
	{
		"Coordinator deals with retractor pin",
		"8/8/4r3/4Q3/4k3/7K/8/8 b 0 -",
		{ 1ULL, 8ULL, 36ULL, 897ULL, 9658ULL, 239135ULL },
		{ 0ULL, 5ULL, 3ULL, 20ULL, 143ULL, 6179ULL },
		{ 0ULL, 5ULL, 3ULL, 20ULL, 143ULL, 6179ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 13ULL }
	},
	{
		"Straddler retractor pin",
		"k7/8/3pKQ2/5p2/8/8/8/8 w 0 -",
		{ 1ULL, 7ULL, 130ULL, 3199ULL, 72219ULL },
		{ 0ULL, 3ULL, 0ULL, 108ULL, 98ULL },
		{ 0ULL, 3ULL, 0ULL, 108ULL, 98ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler and immobilizer pin",
		"k7/8/3pK1U1/5p2/8/5p2/8/8 w 0 -",
		{ 1ULL, 10ULL, 214ULL, 4179ULL, 111926ULL },
		{ 0ULL, 2ULL, 0ULL, 169ULL, 50ULL },
		{ 0ULL, 2ULL, 0ULL, 169ULL, 50ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"No capture blocker",
		"R1N5/4Q3/3Pp2P/4q1B1/4K3/8/7k/5B2 w 0 -",
		{ 1ULL, 91ULL, 1500ULL, 101363ULL },
		{ 0ULL, 3ULL, 75ULL, 6163ULL },
		{ 0ULL, 3ULL, 75ULL, 6175ULL },
		{ 0ULL, 2ULL, 0ULL, 1987ULL }
	}
};

int runTestSuite(void)
{
    int nodes = 0; // so yeah. it's not "move count." it's the number of leaf nodes. oops.

    int fullMatch = 1;

    puts("Running test suite...");
    clock_t start = clock();
    for (int i = 0; i < NUM_TEST_SUITE_POSITIONS; i++)
    {
        puts("---------------------------------------------");
        pos *pos = &positions[i];

        printf("Using test position %s\n", pos->name);
        printf("FEN: %s\n", pos->fen);

        loadFEN(pos->fen);
        prettyPrintBoard();

        int depth = 0;
        while (pos->moveCounts[depth] != 0)
        {
            printf("===Counting at depth %d===\n", depth);
            MoveCounter count = countMoves(depth, 0);

            nodes += count.moves;

            int match = 1;

            // move count
            puts("");
            printf("Move count: %lld\n", count.moves);
            printf("Expected: %lld\n", pos->moveCounts[depth]);
            if (count.moves != pos->moveCounts[depth])
            {
                puts("NO MATCH!");
                match = 0;
            }

            // capture moves
            puts("");
            printf("Capture moves count: %lld\n", count.captureMoves);
            printf("Expected: %lld\n", pos->captureMoves[depth]);
            if (count.captureMoves != pos->captureMoves[depth])
            {
                puts("NO MATCH!");
                match = 0;
            }

            // pieces captured
            puts("");
            printf("Pieces captured count: %lld\n", count.pieceCaptures);
            printf("Expected: %lld\n", pos->pieceCaptures[depth]);
            if (count.pieceCaptures != pos->pieceCaptures[depth])
            {
                puts("NO MATCH!");
                match = 0;
            }

            // checkmates
            puts("");
            printf("Checkmates count: %lld\n", count.checkmates);
            printf("Expected: %lld\n", pos->checkmates[depth]);
            if (count.checkmates != pos->checkmates[depth])
            {
                puts("NO MATCH!");
                match = 0;
            }
            
            puts("");
            if (!match)
            {
                puts("ERROR HERE <------------------------------------------------------------------------------");
                fullMatch = 0;
            }

            depth++;
        }
    }
    puts("---------------------------------------------");
    clock_t end = clock();

    puts("Test suite complete!");
    if (!fullMatch)
    {
        puts("Some results did not match!");
    }
    else
    {
        puts("All test suite positions succeeded!");
    }
    printf("Took %f seconds. Explored %d nodes.\n", (float)(end - start) / CLOCKS_PER_SEC, nodes);

    return fullMatch;
}
