
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
	},
	{
		"Springer pins straddler in between",
		"8/8/8/1N1p1k2/8/8/8/7K b 0 1",
		{ 1ULL, 10ULL, 218ULL, 3094ULL, 75120ULL },
		{ 0ULL, 0ULL, 8ULL, 6ULL, 1297ULL },
		{ 0ULL, 0ULL, 8ULL, 6ULL, 1297ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Springer pins coordinator in between, capture exception",
		"8/8/8/1N1r1k2/8/8/8/7K b 0 1",
		{ 1ULL, 12ULL, 218ULL, 4850ULL, 103617ULL },
		{ 0ULL, 2ULL, 8ULL, 133ULL, 1898ULL },
		{ 0ULL, 2ULL, 8ULL, 133ULL, 1898ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Springer pins coordinator in between, capture exception exception",
		"8/8/8/NN1r1k2/8/8/8/7K b 0 1",
		{ 1ULL, 10ULL, 342ULL, 5934ULL, 225437ULL },
		{ 0ULL, 0ULL, 8ULL, 284ULL, 5214ULL },
		{ 0ULL, 0ULL, 8ULL, 287ULL, 5214ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 48ULL }
	},
	{
		"Springer pins springer/chameleon in between",
		"8/8/8/1N1nk1bN/8/8/8/7K b 0 1",
		{ 1ULL, 26ULL, 812ULL, 18448ULL, 656547ULL },
		{ 0ULL, 1ULL, 21ULL, 498ULL, 20800ULL },
		{ 0ULL, 1ULL, 21ULL, 498ULL, 20800ULL },
		{ 0ULL, 1ULL, 0ULL, 213ULL, 162ULL }
	},
	{
		"Springer pins straddler, capture exception",
		"8/8/8/4k3/8/2p5/1N6/1p5K b 0 1",
		{ 1ULL, 15ULL, 285ULL, 6473ULL, 142172ULL },
		{ 0ULL, 1ULL, 14ULL, 52ULL, 4199ULL },
		{ 0ULL, 1ULL, 14ULL, 52ULL, 4199ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Springer pins straddler, capture exception exception",
		"8/8/8/4k3/8/2p5/1N6/Np5K b 0 1",
		{ 1ULL, 13ULL, 367ULL, 6205ULL, 216065ULL },
		{ 0ULL, 0ULL, 25ULL, 74ULL, 10114ULL },
		{ 0ULL, 0ULL, 25ULL, 74ULL, 10114ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 16ULL }
	},
	{
		"Springer pins coordinator on landing square capture exception",
		"8/8/2N2kr1/8/8/8/8/7K b 0 1",
		{ 1ULL, 7ULL, 154ULL, 2852ULL, 62962ULL },
		{ 0ULL, 1ULL, 6ULL, 88ULL, 949ULL },
		{ 0ULL, 1ULL, 6ULL, 88ULL, 949ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Springer pins immobilizer on landing square",
		"8/8/4Nku1/8/8/8/8/7K b 0 1",
		{ 1ULL, 5ULL, 63ULL, 1171ULL, 23906ULL, 502795ULL },
		{ 0ULL, 1ULL, 2ULL, 14ULL, 234ULL, 2451ULL },
		{ 0ULL, 1ULL, 2ULL, 14ULL, 234ULL, 2451ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 1ULL }
	},
	{
		"Springer pins immobilizer in between",
		"8/8/1Nu2k2/8/8/8/8/7K b 0 1",
		{ 1ULL, 14ULL, 80ULL, 1617ULL, 29456ULL, 619781ULL },
		{ 0ULL, 0ULL, 1ULL, 1ULL, 308ULL, 2542ULL },
		{ 0ULL, 0ULL, 1ULL, 1ULL, 308ULL, 2542ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 11ULL }
	},
	{
		"Springer pins immobilizer immobilized exception",
		"8/8/1uN2k2/8/6n1/8/8/7K b 0 1",
		{ 1ULL, 35ULL, 79ULL, 2727ULL, 16909ULL, 596279ULL },
		{ 0ULL, 0ULL, 2ULL, 18ULL, 210ULL, 5169ULL },
		{ 0ULL, 0ULL, 2ULL, 18ULL, 210ULL, 5169ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 299ULL }
	},
	{
		"Springer double check immobilizer",
		"8/8/3N1k2/7u/3N2n1/8/8/7K b 0 1",
		{ 1ULL, 6ULL, 144ULL, 3319ULL, 137615ULL },
		{ 0ULL, 0ULL, 3ULL, 93ULL, 2134ULL },
		{ 0ULL, 0ULL, 3ULL, 93ULL, 2134ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 4ULL }
	},
	{
		"Springer check",
		"8/8/8/1N4k1/8/8/4b3/7K b 0 1",
		{ 1ULL, 10ULL, 224ULL, 4837ULL, 110059ULL },
		{ 0ULL, 1ULL, 8ULL, 76ULL, 1649ULL },
		{ 0ULL, 1ULL, 8ULL, 76ULL, 1649ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pins immobilizer",
		"8/8/8/2pK1Up1/8/8/8/7k w 0 1",
		{ 1ULL, 13ULL, 162ULL, 4080ULL, 83573ULL },
		{ 0ULL, 1ULL, 6ULL, 63ULL, 299ULL },
		{ 0ULL, 1ULL, 6ULL, 63ULL, 299ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pins retractor twice",
		"4p3/8/8/2pKQ1p1/8/8/8/7k w 0 1",
		{ 1ULL, 7ULL, 199ULL, 5246ULL, 172449ULL },
		{ 0ULL, 1ULL, 0ULL, 119ULL, 633ULL },
		{ 0ULL, 1ULL, 0ULL, 119ULL, 633ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pins springer",
		"4p3/8/3p4/3KQ3/3N3p/8/8/7k w 0 1",
		{ 1ULL, 24ULL, 693ULL, 25778ULL, 783720ULL },
		{ 0ULL, 2ULL, 3ULL, 1067ULL, 3551ULL },
		{ 0ULL, 2ULL, 3ULL, 1067ULL, 3551ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pins retractor, capture reveals discovery",
		"8/8/4p3/2pKQ1p1/8/8/8/7k w 0 1",
		{ 1ULL, 6ULL, 157ULL, 4026ULL, 123303ULL },
		{ 0ULL, 2ULL, 0ULL, 164ULL, 643ULL },
		{ 0ULL, 2ULL, 0ULL, 164ULL, 643ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pins retractor, capture blocks discovery",
		"8/8/4p3/2pKQ3/8/8/4p3/7k w 0 1",
		{ 1ULL, 7ULL, 183ULL, 4580ULL, 139110ULL },
		{ 0ULL, 3ULL, 4ULL, 177ULL, 788ULL },
		{ 0ULL, 3ULL, 4ULL, 177ULL, 788ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pins retractor capture exception",
		"8/8/4p3/2pKQ3/8/8/8/7k w 0 1",
		{ 1ULL, 7ULL, 115ULL, 3198ULL, 67600ULL },
		{ 0ULL, 3ULL, 0ULL, 100ULL, 186ULL },
		{ 0ULL, 3ULL, 0ULL, 100ULL, 186ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pins springer capture exception",
		"8/8/8/2pKN1p1/8/8/8/7k w 0 1",
		{ 1ULL, 9ULL, 168ULL, 4773ULL, 110041ULL },
		{ 0ULL, 2ULL, 0ULL, 281ULL, 342ULL },
		{ 0ULL, 2ULL, 0ULL, 281ULL, 342ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pins springer capture exception exception",
		"8/4p3/8/2pKN1p1/8/8/8/7k w 0 1",
		{ 1ULL, 6ULL, 171ULL, 4208ULL, 136607ULL },
		{ 0ULL, 1ULL, 0ULL, 467ULL, 615ULL },
		{ 0ULL, 1ULL, 0ULL, 467ULL, 615ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pins coordinator capture exception",
		"4p3/8/8/2pKR1pp/8/8/4p3/7k w 0 1",
		{ 1ULL, 9ULL, 376ULL, 10055ULL, 460948ULL },
		{ 0ULL, 5ULL, 0ULL, 1037ULL, 1600ULL },
		{ 0ULL, 5ULL, 0ULL, 1049ULL, 1600ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pins immobilizer but both attacking straddlers can be immobilized",
		"8/4p3/8/2pKU1p1/8/8/8/7k w 0 1",
		{ 1ULL, 7ULL, 183ULL, 4178ULL, 125843ULL },
		{ 0ULL, 1ULL, 0ULL, 129ULL, 459ULL },
		{ 0ULL, 1ULL, 0ULL, 129ULL, 459ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pins immobilizer immobilized exception",
		"8/8/8/2pK2pU/8/8/8/7k w 0 1",
		{ 1ULL, 12ULL, 147ULL, 3088ULL, 59279ULL },
		{ 0ULL, 1ULL, 0ULL, 51ULL, 30ULL },
		{ 0ULL, 1ULL, 0ULL, 51ULL, 30ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddlers pinning three pieces",
		"4p3/4N3/8/2pK1Qp1/8/4N3/8/4p2k w 0 1",
		{ 1ULL, 16ULL, 526ULL, 28605ULL, 1003570ULL },
		{ 0ULL, 4ULL, 9ULL, 1698ULL, 15455ULL },
		{ 0ULL, 4ULL, 9ULL, 1698ULL, 15470ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddlers pinning six pieces",
		"4p3/4N3/3p4/p1pK1Qp1/p1B3Pp/3BN3/3p4/4p2k w 0 1",
		{ 1ULL, 20ULL, 1136ULL, 57244ULL },
		{ 0ULL, 4ULL, 29ULL, 4574ULL },
		{ 0ULL, 4ULL, 29ULL, 4574ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pins chameleon",
		"k7/8/3pKB2/8/8/5p2/8/8 w 0 1",
		{ 1ULL, 9ULL, 201ULL, 4880ULL, 124893ULL },
		{ 0ULL, 1ULL, 0ULL, 100ULL, 94ULL },
		{ 0ULL, 1ULL, 0ULL, 100ULL, 94ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Retractor pins chameleon capture exception",
		"8/8/4b3/4Q3/4k3/8/8/7K b 0 1",
		{ 1ULL, 5ULL, 83ULL, 2133ULL, 47441ULL },
		{ 0ULL, 2ULL, 3ULL, 33ULL, 310ULL },
		{ 0ULL, 2ULL, 3ULL, 33ULL, 310ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Retractor pins immobilizer",
		"8/8/1b6/4Q3/3uk3/8/8/7K b 0 1",
		{ 1ULL, 28ULL, 83ULL, 2542ULL, 17587ULL, 601022ULL },
		{ 0ULL, 1ULL, 0ULL, 81ULL, 44ULL, 12813ULL },
		{ 0ULL, 1ULL, 0ULL, 81ULL, 44ULL, 12813ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 319ULL }
	},
	{
		"Retractor pins chameleon capture exception exception by straddler",
		"8/8/4b3/2P1Q3/4k3/4P3/8/7K b 0 1",
		{ 1ULL, 3ULL, 91ULL, 2270ULL, 79169ULL },
		{ 0ULL, 2ULL, 2ULL, 77ULL, 465ULL },
		{ 0ULL, 2ULL, 2ULL, 77ULL, 465ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Retractor pins chameleon capture exception exception by coordinator",
		"8/8/4b3/R3Q3/B3k3/8/8/4p2K b 0 1",
		{ 1ULL, 4ULL, 191ULL, 5313ULL, 279337ULL },
		{ 0ULL, 1ULL, 3ULL, 155ULL, 7499ULL },
		{ 0ULL, 1ULL, 3ULL, 155ULL, 7523ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 328ULL }
	},
	{
		"Retractor pins coordinator capture exception",
		"8/8/4r3/4Q3/4k3/8/8/7K b 0 1",
		{ 1ULL, 8ULL, 92ULL, 2418ULL, 47259ULL },
		{ 0ULL, 5ULL, 3ULL, 77ULL, 296ULL },
		{ 0ULL, 5ULL, 3ULL, 77ULL, 296ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Retractor pins coordinator partial capture exception exception by coordinator",
		"8/8/4r3/R3Q3/B3k3/8/8/4p2K b 0 1",
		{ 1ULL, 7ULL, 282ULL, 7507ULL, 358913ULL },
		{ 0ULL, 4ULL, 4ULL, 348ULL, 11251ULL },
		{ 0ULL, 4ULL, 4ULL, 348ULL, 11292ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 142ULL }
	},
	{
		"Retractor pins straddler capture exception",
		"8/8/6p1/4pQ2/4k3/8/8/7K b 0 1",
		{ 1ULL, 14ULL, 245ULL, 6159ULL, 133588ULL },
		{ 0ULL, 2ULL, 11ULL, 84ULL, 2425ULL },
		{ 0ULL, 2ULL, 11ULL, 84ULL, 2425ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Retractor pins straddler capture exception exception by springer",
		"8/7N/6p1/4pQ2/4k3/8/8/7K b 0 1",
		{ 1ULL, 13ULL, 420ULL, 7853ULL, 279569ULL },
		{ 0ULL, 1ULL, 12ULL, 198ULL, 10207ULL },
		{ 0ULL, 1ULL, 12ULL, 198ULL, 10207ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 18ULL }
	},
	{
		"Retractor pins straddler capture exception exception by coordinator",
		"4B3/8/4R1p1/4pQ2/2p1k3/8/8/4K3 b 0 1",
		{ 1ULL, 10ULL, 406ULL, 8150ULL, 366614ULL },
		{ 0ULL, 1ULL, 15ULL, 246ULL, 15826ULL },
		{ 0ULL, 1ULL, 15ULL, 246ULL, 15882ULL },
		{ 0ULL, 0ULL, 1ULL, 0ULL, 477ULL }
	},
	{
		"Retractor pins straddler capture exception exception by straddler",
		"5P2/8/6p1/4pQ2/3Pk3/8/8/4K3 b 0 1",
		{ 1ULL, 13ULL, 491ULL, 9241ULL, 357811ULL },
		{ 0ULL, 2ULL, 12ULL, 615ULL, 6253ULL },
		{ 0ULL, 2ULL, 12ULL, 615ULL, 6253ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Retractor check",
		"8/8/1b6/4Q3/4k3/8/8/7K b 0 1",
		{ 1ULL, 5ULL, 112ULL, 2666ULL, 60471ULL },
		{ 0ULL, 1ULL, 0ULL, 39ULL, 346ULL },
		{ 0ULL, 1ULL, 0ULL, 39ULL, 346ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Coordinator pins capture exception",
		"8/pnp3k1/1R6/8/8/8/8/6K1 b 0 1",
		{ 1ULL, 6ULL, 110ULL, 3256ULL, 70931ULL },
		{ 0ULL, 1ULL, 0ULL, 50ULL, 1055ULL },
		{ 0ULL, 1ULL, 0ULL, 50ULL, 1055ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 19ULL }
	},
	{
		"Coordinator pins three pieces from a distance",
		"8/8/1b1q1bk1/8/3R4/8/6K1/8 b 0 1",
		{ 1ULL, 9ULL, 240ULL, 9188ULL, 225102ULL },
		{ 0ULL, 0ULL, 0ULL, 126ULL, 4699ULL },
		{ 0ULL, 0ULL, 0ULL, 126ULL, 4714ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Coordinator pins five pieces at once",
		"8/8/Bb1q1bk1/8/3R1p2/4n3/6K1/8 b 0 1",
		{ 1ULL, 14ULL, 385ULL, 17517ULL, 530597ULL },
		{ 0ULL, 1ULL, 21ULL, 350ULL, 25438ULL },
		{ 0ULL, 1ULL, 22ULL, 350ULL, 26354ULL },
		{ 0ULL, 0ULL, 0ULL, 2ULL, 256ULL }
	},
	{
		"Coordinator check, can be immobilized or blocked",
		"8/8/Bb1q1bk1/8/3R1p2/8/u5K1/4p3 b 0 1",
		{ 1ULL, 11ULL, 352ULL, 19542ULL, 629428ULL },
		{ 0ULL, 0ULL, 48ULL, 59ULL, 59650ULL },
		{ 0ULL, 0ULL, 49ULL, 59ULL, 60944ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 167ULL }
	},
	{
		"Coordinator check, can be immobilized but not blocked",
		"8/8/Bb3bk1/8/3R1p2/8/u5K1/4p3 b 0 1",
		{ 1ULL, 7ULL, 235ULL, 12872ULL, 438634ULL },
		{ 0ULL, 0ULL, 31ULL, 54ULL, 40601ULL },
		{ 0ULL, 0ULL, 31ULL, 54ULL, 41411ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 182ULL }
	},
	{
		"Coordinator pins two straddlers horizontally",
		"8/1pRp2k1/8/8/8/8/6K1/8 b 0 1",
		{ 1ULL, 4ULL, 96ULL, 1706ULL, 45859ULL },
		{ 0ULL, 0ULL, 0ULL, 1ULL, 658ULL },
		{ 0ULL, 0ULL, 0ULL, 1ULL, 658ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 26ULL }
	},
	{
		"Chameleon pins capture exception",
		"8/1pnp3k/2B5/8/8/8/7R/6K1 b 0 1",
		{ 1ULL, 2ULL, 62ULL, 2114ULL, 70096ULL },
		{ 0ULL, 1ULL, 1ULL, 36ULL, 1108ULL },
		{ 0ULL, 1ULL, 1ULL, 36ULL, 1108ULL },
		{ 0ULL, 0ULL, 1ULL, 0ULL, 1584ULL }
	},
	{
		"King pins pieces",
		"8/1ppp3k/2K5/8/8/8/7R/8 b 0 1",
		{ 1ULL, 3ULL, 80ULL, 1651ULL, 39373ULL, 1018172ULL },
		{ 0ULL, 0ULL, 12ULL, 6ULL, 5064ULL, 3592ULL },
		{ 0ULL, 0ULL, 12ULL, 6ULL, 5410ULL, 3592ULL },
		{ 0ULL, 0ULL,  0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"King and chameleon pin capture exception exception",
		"8/1pppnqnk/2K2B2/8/8/8/7R/8 b 0 1",
		{ 1ULL, 2ULL, 84ULL, 4116ULL, 142135ULL },
		{ 0ULL, 0ULL, 13ULL, 173ULL, 19228ULL },
		{ 0ULL, 0ULL, 13ULL, 173ULL, 21104ULL },
		{ 0ULL, 0ULL, 2ULL, 0ULL, 2059ULL }
	},
	{
		"Chameleons pin six pieces",
		"8/1pppnqnk/2B2B2/8/8/8/7R/K7 b 0 1",
		{ 1ULL, 5ULL, 220ULL, 11444ULL, 506641ULL },
		{ 0ULL, 3ULL, 5ULL, 321ULL, 15023ULL },
		{ 0ULL, 3ULL, 5ULL, 321ULL, 15023ULL },
		{ 0ULL, 0ULL, 2ULL, 0ULL, 3250ULL }
	},
	{
		"Chameleons pin six pieces, one of them is on the other side",
		"5B2/1pppnqnk/2B5/8/8/8/7R/K7 b 0 1",
		{ 1ULL, 1ULL, 36ULL, 1888ULL, 67650ULL },
		{ 0ULL, 1ULL, 0ULL, 12ULL, 868ULL },
		{ 0ULL, 1ULL, 0ULL, 12ULL, 868ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 51ULL }
	},
	{
		"Chameleon check but can be immobilized",
		"8/8/1u6/4B3/4k3/8/8/7K b 0 1",
		{ 1ULL, 8ULL, 101ULL, 2055ULL, 40765ULL },
		{ 0ULL, 1ULL, 0ULL, 25ULL, 0ULL },
		{ 0ULL, 1ULL, 0ULL, 25ULL, 0ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"King checks but both kings are immobilized",
		"P7/8/8/3UK3/4ku2/8/8/p7 b 0 1",
		{ 1ULL, 15ULL, 211ULL, 2956ULL, 41471ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Double chameleon attack but they're both immobilized",
		"p7/8/8/4B3/3uk3/4B3/8/7K b 0 1",
		{ 1ULL, 21ULL, 62ULL, 1210ULL, 6667ULL, 125024ULL },
		{ 0ULL, 2ULL, 0ULL, 101ULL, 26ULL, 8225ULL },
		{ 0ULL, 2ULL, 0ULL, 101ULL, 26ULL, 8225ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Enemy straddler pinned",
		"8/8/8/2pKpp2/4Q3/8/8/7k w 0 1",
		{ 1ULL, 20ULL, 479ULL, 8330ULL, 243074ULL },
		{ 0ULL, 2ULL, 1ULL, 948ULL, 556ULL },
		{ 0ULL, 2ULL, 1ULL, 948ULL, 556ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"K-C double capture reveals discovery",
		"8/8/p1p1k2r/8/2R5/4p3/3pK2P/8 w 0 1",
		{ 1ULL, 25ULL, 652ULL, 15620ULL, 703810ULL },
		{ 0ULL, 5ULL, 5ULL, 1735ULL, 5560ULL },
		{ 0ULL, 5ULL, 5ULL, 1785ULL, 5598ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler and chameleon double capture reveals discovery",
		"3B4/8/B3k2r/8/8/P3pP1P/3pK3/3P4 w 0 1",
		{ 1ULL, 60ULL, 1626ULL, 79796ULL },
		{ 0ULL, 2ULL, 27ULL, 3342ULL },
		{ 0ULL, 2ULL, 27ULL, 3800ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Retractor to c1 reveals discovery",
		"8/8/2r1k3/8/4p3/2pp4/2Q1K3/8 w 0 1",
		{ 1ULL, 14ULL, 645ULL, 8843ULL, 432134ULL },
		{ 0ULL, 1ULL, 0ULL, 693ULL, 1038ULL },
		{ 0ULL, 1ULL, 0ULL, 693ULL, 1038ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Springer to c1 reveals discovery",
		"8/4k3/2r5/8/2N1p3/8/2p1K3/8 w 0 1",
		{ 1ULL, 21ULL, 687ULL, 10542ULL, 390771ULL },
		{ 0ULL, 1ULL, 9ULL, 888ULL, 4662ULL },
		{ 0ULL, 1ULL, 9ULL, 888ULL, 4662ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 1ULL }
	},
	{
		"Retractor to e4 reveals discovery",
		"4p3/8/4p3/2pKQ3/8/8/8/7k w 0 1",
		{ 1ULL, 6ULL, 146ULL, 3888ULL, 117923ULL },
		{ 0ULL, 2ULL, 0ULL, 147ULL, 367ULL },
		{ 0ULL, 2ULL, 0ULL, 147ULL, 367ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Retractor to e5 blocks discovery",
		"4p3/4p3/4Q3/2pK4/8/8/8/7k w 0 1",
		{ 1ULL, 7ULL, 185ULL, 4826ULL, 153515ULL },
		{ 0ULL, 2ULL, 4ULL, 224ULL, 721ULL },
		{ 0ULL, 2ULL, 4ULL, 224ULL, 721ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pins coordinator capture exception exception horizontal",
		"8/2P5/8/2pKR1pp/8/2P5/8/7k w 0 1",
		{ 1ULL, 27ULL, 586ULL, 21482ULL, 574293ULL },
		{ 0ULL, 3ULL, 0ULL, 2468ULL, 1294ULL },
		{ 0ULL, 3ULL, 0ULL, 2468ULL, 1294ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pins coordinator capture exception exception vertical",
		"4p3/2P5/8/2pKR1p1/8/2P5/8/7k w 0 1",
		{ 1ULL, 26ULL, 668ULL, 23722ULL, 674334ULL },
		{ 0ULL, 3ULL, 0ULL, 2363ULL, 2646ULL },
		{ 0ULL, 3ULL, 0ULL, 2381ULL, 2646ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Coordinator pins springers capture exception",
		"8/8/1n1nk3/8/1R6/8/4K3/8 b 0 1",
		{ 1ULL, 10ULL, 165ULL, 5288ULL, 96401ULL },
		{ 0ULL, 2ULL, 0ULL, 143ULL, 2273ULL },
		{ 0ULL, 2ULL, 0ULL, 143ULL, 2273ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Coordinator pins springer, straddler, retractor capture exceptions",
		"8/8/nqp1k3/pR6/8/4K3/8/8 b 0 1",
		{ 1ULL, 12ULL, 225ULL, 6800ULL, 132068ULL },
		{ 0ULL, 3ULL, 2ULL, 253ULL, 2733ULL },
		{ 0ULL, 3ULL, 2ULL, 253ULL, 2736ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Rxc4 causes double capture and moves out of the way",
		"8/7r/p2pk3/8/4p3/3R4/2p1K2P/8 w 0 1",
		{ 1ULL, 21ULL, 556ULL, 12983ULL, 614818ULL },
		{ 0ULL, 2ULL, 7ULL, 780ULL, 6831ULL },
		{ 0ULL, 2ULL, 7ULL, 827ULL, 6846ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Retractor to b7 reveals discovery",
		"8/8/1q2kp2/1P6/8/4K3/1R6/8 b 0 1",
		{ 1ULL, 17ULL, 533ULL, 9939ULL, 320208ULL },
		{ 0ULL, 0ULL, 1ULL, 181ULL, 3181ULL },
		{ 0ULL, 0ULL, 1ULL, 181ULL, 3183ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Springer b7 reveals discovery",
		"8/8/1P2kp2/8/1n6/4K3/1R6/8 b 0 1",
		{ 1ULL, 26ULL, 538ULL, 10899ULL, 258153ULL },
		{ 0ULL, 1ULL, 32ULL, 365ULL, 8234ULL },
		{ 0ULL, 1ULL, 32ULL, 365ULL, 8313ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler blocks a pinned enemy piece but blocks the attack",
		"8/8/3rk3/8/8/P5P1/3pK3/3P4 w 0 1",
		{ 1ULL, 25ULL, 742ULL, 16410ULL, 543972ULL },
		{ 0ULL, 2ULL, 3ULL, 777ULL, 4860ULL },
		{ 0ULL, 2ULL, 3ULL, 777ULL, 4872ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Retractor backward captures but still blocks",
		"8/8/3rk3/8/3p4/3Q2P1/4K3/8 w 0 1",
		{ 1ULL, 22ULL, 655ULL, 13848ULL, 463117ULL },
		{ 0ULL, 1ULL, 5ULL, 192ULL, 6542ULL },
		{ 0ULL, 1ULL, 5ULL, 192ULL, 6557ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Springer with piece in between but landing square blocked",
		"8/8/N3nkn1/8/8/8/8/7K b 0 1",
		{ 1ULL, 44ULL, 853ULL, 29891ULL, 664475ULL },
		{ 0ULL, 0ULL, 15ULL, 297ULL, 15627ULL },
		{ 0ULL, 0ULL, 15ULL, 297ULL, 15627ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Springer with piece in between but no landing square",
		"8/8/N5pk/8/8/8/8/7K b 0 1",
		{ 1ULL, 16ULL, 353ULL, 5769ULL, 141362ULL },
		{ 0ULL, 0ULL, 11ULL, 14ULL, 2049ULL },
		{ 0ULL, 0ULL, 11ULL, 14ULL, 2049ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Springer with piece in between and at landing square",
		"8/8/1N1n1kn1/8/8/8/8/7K b 0 1",
		{ 1ULL, 45ULL, 932ULL, 31017ULL, 690023ULL },
		{ 0ULL, 1ULL, 31ULL, 415ULL, 18677ULL },
		{ 0ULL, 1ULL, 31ULL, 415ULL, 18677ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Straddler pin but check straddlers immobilized",
		"8/4p3/5U2/2pKN1p1/8/8/8/7k w 0 1",
		{ 1ULL, 41ULL, 654ULL, 23433ULL, 528855ULL },
		{ 0ULL, 3ULL, 4ULL, 1215ULL, 3634ULL },
		{ 0ULL, 3ULL, 4ULL, 1215ULL, 3634ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Coordinator pin but springer can still take straddler",
		"8/8/4kp2/1P6/1n6/4K3/1R6/8 b 0 1",
		{ 1ULL, 26ULL, 558ULL, 11681ULL, 289556ULL },
		{ 0ULL, 2ULL, 32ULL, 373ULL, 9320ULL },
		{ 0ULL, 2ULL, 32ULL, 373ULL, 9416ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Coordinator pin but retractor can still take straddler",
		"8/8/4kp2/1q6/1P6/4K3/1R6/8 b 0 1",
		{ 1ULL, 26ULL, 790ULL, 14394ULL, 446981ULL },
		{ 0ULL, 1ULL, 8ULL, 173ULL, 6972ULL },
		{ 0ULL, 1ULL, 8ULL, 173ULL, 6989ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
	},
	{
		"Coordinator pins but is immobilized",
		"8/8/1b3bk1/8/3R4/2u5/6K1/8 b 0 1",
		{ 1ULL, 44ULL, 342ULL, 19183ULL, 187016ULL },
		{ 0ULL, 0ULL, 12ULL, 0ULL, 7399ULL },
		{ 0ULL, 0ULL, 12ULL, 0ULL, 7505ULL },
		{ 0ULL, 0ULL, 0ULL, 2ULL, 0ULL }
	},
	{
		"Coordinator pins (and checks) but is immobilized",
		"8/8/Bb3bk1/8/3R4/2u5/6K1/8 b 0 1",
		{ 1ULL, 43ULL, 927ULL, 42152ULL },
		{ 0ULL, 0ULL, 12ULL, 0ULL },
		{ 0ULL, 0ULL, 12ULL, 0ULL },
		{ 0ULL, 0ULL, 0ULL, 2ULL }
	},
	{
		"Straddlers are both immobilized and cannot move to capture",
		"k7/5p2/3pKBU1/5p2/8/5p2/8/8 w 0 1",
		{ 1ULL, 26ULL, 673ULL, 17576ULL, 501595ULL },
		{ 0ULL, 3ULL, 2ULL, 1466ULL, 3825ULL },
		{ 0ULL, 3ULL, 2ULL, 1466ULL, 3843ULL },
		{ 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }
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
