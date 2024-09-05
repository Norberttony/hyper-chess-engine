
#include "make-unmake.h"


void makeMove(Move m)
{
    // decode move
    int type = m & 0b111;
    int from = (m >> 3) & 0b111111;
    int to = (m >> 9) & 0b111111;

    int c1 = (m >> 15) & 0b111;
    int c2 = (m >> 18) & 0b111;
    int c3 = (m >> 21) & 0b111;
    int c4 = (m >> 24) & 0b111;

    int coordinateSq;

    U64 zobristHashUpdate = 0ULL;

    // interpret capture bits
    switch(type)
    {
        case straddler:
            coordinateSq = 0;
            int upSq = to - 8;
            int ltSq = to - 1;
            int rtSq = to + 1;
            int dnSq = to + 8;

            // update zobrist hash as needed (remove captured pieces)
            zobristHashUpdate ^=
                ((c1 != 0) * get_zobrist_hash(upSq, c1, toPlay)) ^
                ((c2 != 0) * get_zobrist_hash(ltSq, c2, toPlay)) ^
                ((c3 != 0) * get_zobrist_hash(rtSq, c3, toPlay)) ^
                ((c4 != 0) * get_zobrist_hash(dnSq, c4, toPlay));

            // up
            position[notToPlay + c1] ^= 1ULL * (c1 > 0) << upSq;
            position[notToPlay] ^= 1ULL * (c1 > 0) << upSq;
            pieceList[upSq] = pieceList[upSq] * (c1 == 0);

            // left
            position[notToPlay + c2] ^= 1ULL * (c2 > 0) << ltSq;
            position[notToPlay] ^= 1ULL * (c2 > 0) << ltSq;
            pieceList[ltSq] = pieceList[ltSq] * (c2 == 0);

            // right
            position[notToPlay + c3] ^= 1ULL * (c3 > 0) << rtSq;
            position[notToPlay] ^= 1ULL * (c3 > 0) << rtSq;
            pieceList[rtSq] = pieceList[rtSq] * (c3 == 0);

            // down
            position[notToPlay + c4] ^= 1ULL * (c4 > 0) << dnSq;
            position[notToPlay] ^= 1ULL * (c4 > 0) << dnSq;
            pieceList[to + 8] = pieceList[to + 8] * (c4 == 0);

            break;
        
        case immobilizer:
            break;

        case coordinator:
            coordinateSq = pop_lsb(position[toPlay + king]);
            
            // top death square
            int top = pop_lsb(deathSquares[coordinateSq][to][0]);
            position[notToPlay + c1] ^= 1ULL * (c1 > 0) << top;
            position[notToPlay]      ^= 1ULL * (c1 > 0) << top;
            pieceList[top] *= c1 == 0;

            // bottom death square
            int bottom = pop_lsb(deathSquares[coordinateSq][to][1]);
            position[notToPlay + c2] ^= 1ULL * (c2 > 0) << bottom;
            position[notToPlay]      ^= 1ULL * (c2 > 0) << bottom;
            pieceList[bottom] *= c2 == 0;

            // update zobrist hash by removing captured pieces
            zobristHashUpdate ^= 
                ((c1 != 0) * get_zobrist_hash(top, c1, toPlay)) ^
                ((c2 != 0) * get_zobrist_hash(bottom, c2, toPlay));

            break;

        case king:

            // can capture by displacement
            // assumes pieceList will be updated by king overwriting square
            position[notToPlay + c1] ^= 1ULL * (c1 > 0) << to;
            position[notToPlay]      ^= 1ULL * (c1 > 0) << to;

            zobristHashUpdate ^= (c1 != 0) * get_zobrist_hash(to, c1, toPlay);

            // can form death squares with own coordinator
            coordinateSq = pop_lsb(position[toPlay + coordinator]);

            // top death square
            int deathSq = pop_lsb(deathSquares[coordinateSq][to][0]);
            zobristHashUpdate ^= (c2 != 0) * get_zobrist_hash(deathSq, c2, toPlay);
            position[notToPlay + c2] ^= 1ULL * (c2 > 0) << deathSq;
            position[notToPlay]      ^= 1ULL * (c2 > 0) << deathSq;
            pieceList[deathSq] = pieceList[deathSq] * (c2 == 0);

            // bottom death square
            deathSq = pop_lsb(deathSquares[coordinateSq][to][1]);
            zobristHashUpdate ^= (c3 != 0) * get_zobrist_hash(deathSq, c3, toPlay);
            position[notToPlay + c3] ^= 1ULL * (c3 > 0) << deathSq;
            position[notToPlay]      ^= 1ULL * (c3 > 0) << deathSq;
            pieceList[deathSq] = pieceList[deathSq] * (c3 == 0);

            // can form death squares with chameleons (only against coordinator)
            U64 chamBoard = position[toPlay + chameleon];
            int cham1 = pop_lsb(chamBoard);
            int cham2 = pop_lsb(chamBoard - 1 & chamBoard);

            // to-do: combine all of the boards into one and then apply them.
            int isDeath = (m & move_kingc1mask) > 0;
            deathSq = pop_lsb(deathSquares[cham1][to][0]);
            position[notToPlay + coordinator] ^= 1ULL * isDeath << deathSq;
            position[notToPlay]               ^= 1ULL * isDeath << deathSq;
            pieceList[deathSq] *= (isDeath == 0);

            zobristHashUpdate ^= (isDeath != 0) * get_zobrist_hash(deathSq, coordinator, toPlay);

            isDeath = m & move_kingc2mask;
            deathSq = pop_lsb(deathSquares[cham1][to][1]);
            position[notToPlay + coordinator] ^= 1ULL * (isDeath > 0) << deathSq;
            position[notToPlay]               ^= 1ULL * (isDeath > 0) << deathSq;
            pieceList[deathSq] *= (isDeath == 0);

            zobristHashUpdate ^= (isDeath != 0) * get_zobrist_hash(deathSq, coordinator, toPlay);

            isDeath = m & move_kingc3mask;
            deathSq = pop_lsb(deathSquares[cham2][to][0]);
            position[notToPlay + coordinator] ^= 1ULL * (isDeath > 0) << deathSq;
            position[notToPlay]               ^= 1ULL * (isDeath > 0) << deathSq;
            pieceList[deathSq] *= (isDeath == 0);

            zobristHashUpdate ^= (isDeath != 0) * get_zobrist_hash(deathSq, coordinator, toPlay);

            isDeath = m & move_kingc4mask;
            deathSq = pop_lsb(deathSquares[cham2][to][1]);
            position[notToPlay + coordinator] ^= 1ULL * (isDeath > 0) << deathSq;
            position[notToPlay]               ^= 1ULL * (isDeath > 0) << deathSq;
            pieceList[deathSq] *= (isDeath == 0);

            zobristHashUpdate ^= (isDeath != 0) * get_zobrist_hash(deathSq, coordinator, toPlay);

            break;

        case springer:

            // just a garbage variable... coordinateSq in this case is where the springer captured
            // a piece at
            coordinateSq = pop_lsb(springerCaptures[from][to]);

            position[notToPlay + c1] ^= 1ULL * (c1 > 0) << coordinateSq;
            position[notToPlay]      ^= 1ULL * (c1 > 0) << coordinateSq;
            pieceList[coordinateSq] *= (c1 == 0);

            zobristHashUpdate ^= (c1 != 0) * get_zobrist_hash(coordinateSq, c1, toPlay);

            break;
        
        case retractor:

            // coordinateSq in this case is where the retractor captured a piece at
            coordinateSq = pop_lsb(retractorCaptures[from][to]);

            position[notToPlay + c1] ^= 1ULL * (c1 > 0) << coordinateSq;
            position[notToPlay]      ^= 1ULL * (c1 > 0) << coordinateSq;
            pieceList[coordinateSq]  *= (c1 == 0);

            zobristHashUpdate ^= (c1 != 0) * get_zobrist_hash(coordinateSq, c1, toPlay);

            break;

        case chameleon:
            // straddler moves
            c1 = (m >> 15) & 1;
            c2 = (m >> 16) & 1;
            c3 = (m >> 17) & 1;
            c4 = (m >> 18) & 1;

            // up
            position[notToPlay + straddler] ^= 1ULL * c1 << (to - 8);
            position[notToPlay]             ^= 1ULL * c1 << (to - 8);
            pieceList[to - 8] *= !c1;

            zobristHashUpdate ^= c1 * get_zobrist_hash(to - 8, c1, toPlay);

            // left
            position[notToPlay + straddler] ^= 1ULL * c2 << (to - 1);
            position[notToPlay]             ^= 1ULL * c2 << (to - 1);
            pieceList[to - 1] *= !c2;

            zobristHashUpdate ^= c2 * get_zobrist_hash(to - 1, c2, toPlay);

            // right
            position[notToPlay + straddler] ^= 1ULL * c3 << (to + 1);
            position[notToPlay]             ^= 1ULL * c3 << (to + 1);
            pieceList[to + 1] *= !c3;

            zobristHashUpdate ^= c3 * get_zobrist_hash(to + 1, c3, toPlay);

            // down
            position[notToPlay + straddler] ^= 1ULL * c4 << (to + 8);
            position[notToPlay]             ^= 1ULL * c4 << (to + 8);
            pieceList[to + 8] *= !c4;

            zobristHashUpdate ^= c4 * get_zobrist_hash(to + 8, c4, toPlay);

            // consider coordinator moves
            coordinateSq = pop_lsb(position[toPlay + king]);
            c1 = (m >> 19) & 1;
            U64 death = deathSquares[to][coordinateSq][0];
            int captureSq = pop_lsb(death);
            position[notToPlay + coordinator]   ^= death * c1;
            position[notToPlay]                 ^= death * c1;
            pieceList[captureSq] *= !c1;

            zobristHashUpdate ^= c1 * get_zobrist_hash(captureSq, coordinator, toPlay);

            // other death square for coordinator
            c1 = (m >> 20) & 1;
            death = deathSquares[to][coordinateSq][1];
            captureSq = pop_lsb(death);
            position[notToPlay + coordinator]   ^= death * c1;
            position[notToPlay]                 ^= death * c1;
            pieceList[captureSq] *= !c1;

            zobristHashUpdate ^= c1 * get_zobrist_hash(captureSq, coordinator, toPlay);

            // consider retractor moves
            c1 = (m >> 21) & 1;
            captureSq = pop_lsb(retractorCaptures[from][to]);
            position[notToPlay + retractor] ^= 1ULL * c1 << captureSq;
            position[notToPlay]             ^= 1ULL * c1 << captureSq;
            pieceList[captureSq] *= !c1;

            zobristHashUpdate ^= c1 * get_zobrist_hash(captureSq, retractor, toPlay);

            // consider springer moves
            c1 = (m >> 22) & 1;
            captureSq = pop_lsb(springerCaptures[from][to]);
            position[notToPlay + springer]  ^= 1ULL * c1 << captureSq;
            position[notToPlay]             ^= 1ULL * c1 << captureSq;
            pieceList[captureSq] *= !c1;

            zobristHashUpdate ^= c1 * get_zobrist_hash(captureSq, springer, toPlay);

            break;
    }

    // move piece
    U64 toggle = (1ULL << from) | (1ULL << to);
    position[toPlay + type] ^= toggle;
    position[toPlay] ^= toggle;

    // remove piece from zobrist hash
    zobristHashUpdate ^= get_zobrist_hash(from, type, !toPlay);
    // add piece back
    zobristHashUpdate ^= get_zobrist_hash(to, type, !toPlay);

    // update piece list
    pieceList[to] = pieceList[from];
    pieceList[from] = 0;

    // toggle turn
    toPlay = !toPlay * 8;
    notToPlay = !notToPlay * 8;

    // toggle turn on zobrist hash
    zobristHashUpdate ^= zobristHashes[ZOBRIST_HASH_COUNT - 1];

    // apply zobrist hash updates
    zobristHash ^= zobristHashUpdate;

    // add to repeat table
    repeatTable[repeatTableIndex++] = zobristHash;
    repeatTableIndex -= REPEAT_TABLE_ENTRIES * (repeatTableIndex == REPEAT_TABLE_ENTRIES);

    // update halfmove counter
    halfmove++;
}

void unmakeMove(Move m)
{
    // decode move
    int type = m & 0b111;
    int from = (m >> 3) & 0b111111;
    int to = (m >> 9) & 0b111111;

    int c1 = (m >> 15) & 0b111;
    int c2 = (m >> 18) & 0b111;
    int c3 = (m >> 21) & 0b111;
    int c4 = (m >> 24) & 0b111;

    int coordinateSq;
    U64 zobristHashUpdate = 0ULL;

    // update halfmove counter
    halfmove--;

    // remove from repeat table
    repeatTableIndex += REPEAT_TABLE_ENTRIES * (repeatTableIndex == 0);
    repeatTable[--repeatTableIndex] = 0ULL;

    // toggle turn on zobrist hash
    zobristHashUpdate ^= zobristHashes[ZOBRIST_HASH_COUNT - 1];

    // toggle turn
    toPlay = !toPlay * 8;
    notToPlay = !notToPlay * 8;

    // remove piece from zobrist hash
    zobristHashUpdate ^= get_zobrist_hash(to, type, !toPlay);
    // add piece back
    zobristHashUpdate ^= get_zobrist_hash(from, type, !toPlay);

    // unmove piece
    U64 toggle = (1ULL << from) | (1ULL << to);
    position[toPlay + type] ^= toggle;
    position[toPlay] ^= toggle;

    // update piece list
    pieceList[from] = pieceList[to];
    pieceList[to] = 0;

    // interpret capture bits
    switch(type)
    {
        case straddler:
            coordinateSq = 0;
            int upSq = to - 8;
            int ltSq = to - 1;
            int rtSq = to + 1;
            int dnSq = to + 8;

            // update zobrist hash as needed (add back captured pieces)
            zobristHashUpdate ^=
                ((c1 != 0) * get_zobrist_hash(upSq, c1, toPlay)) ^
                ((c2 != 0) * get_zobrist_hash(ltSq, c2, toPlay)) ^
                ((c3 != 0) * get_zobrist_hash(rtSq, c3, toPlay)) ^
                ((c4 != 0) * get_zobrist_hash(dnSq, c4, toPlay));

            // up
            position[notToPlay + c1] |= 1ULL * (c1 > 0) << (to - 8);
            position[notToPlay] |= 1ULL * (c1 > 0) << (to - 8);
            pieceList[to - 8] += c1; // assumes that, after a capture, the piece at this square would be gone.

            // left
            position[notToPlay + c2] |= 1ULL * (c2 > 0) << (to - 1);
            position[notToPlay] |= 1ULL * (c2 > 0) << (to - 1);
            pieceList[to - 1] += c2;

            // right
            position[notToPlay + c3] |= 1ULL * (c3 > 0) << (to + 1);
            position[notToPlay] |= 1ULL * (c3 > 0) << (to + 1);
            pieceList[to + 1] += c3;

            // down
            position[notToPlay + c4] |= 1ULL * (c4 > 0) << (to + 8);
            position[notToPlay] |= 1ULL * (c4 > 0) << (to + 8);
            pieceList[to + 8] += c4;
            break;

        case immobilizer:
            break;
        
        case coordinator:
            coordinateSq = pop_lsb(position[toPlay + king]);
            
            // top death square
            int top = pop_lsb(deathSquares[coordinateSq][to][0]);
            position[notToPlay + c1] |= 1ULL * (c1 > 0) << top;
            position[notToPlay]      |= 1ULL * (c1 > 0) << top;
            pieceList[top] += c1;

            // bottom death square
            int bottom = pop_lsb(deathSquares[coordinateSq][to][1]);
            position[notToPlay + c2] |= 1ULL * (c2 > 0) << bottom;
            position[notToPlay]      |= 1ULL * (c2 > 0) << bottom;
            pieceList[bottom] += c2;

            zobristHashUpdate ^=
                ((c1 != 0) * get_zobrist_hash(top, c1, toPlay)) ^
                ((c2 != 0) * get_zobrist_hash(bottom, c2, toPlay));
            
            break;

        case king:

            // capture by displacement
            position[notToPlay + c1] |= 1ULL * (c1 > 0) << to;
            position[notToPlay]      |= 1ULL * (c1 > 0) << to;
            pieceList[to] += c1;

            // uncapture piece on zobrist hash
            zobristHashUpdate ^= (c1 != 0) * get_zobrist_hash(to, c1, toPlay);

            // death squares with coordinator
            // can form death squares with own coordinator
            coordinateSq = pop_lsb(position[toPlay + coordinator]);

            int deathSq = pop_lsb(deathSquares[coordinateSq][to][0]);
            position[notToPlay + c2] |= 1ULL * (c2 > 0) << deathSq;
            position[notToPlay]      |= 1ULL * (c2 > 0) << deathSq;
            pieceList[deathSq] += c2;
            
            zobristHashUpdate ^= (c2 != 0) * get_zobrist_hash(deathSq, c2, toPlay);

            deathSq = pop_lsb(deathSquares[coordinateSq][to][1]);
            position[notToPlay + c3] |= 1ULL * (c3 > 0) << deathSq;
            position[notToPlay]      |= 1ULL * (c3 > 0) << deathSq;
            pieceList[deathSq] += c3;

            zobristHashUpdate ^= (c3 != 0) * get_zobrist_hash(deathSq, c3, toPlay);

            // consider king-chameleon duo
            U64 chamBoard = position[toPlay + chameleon];
            int cham1 = pop_lsb(chamBoard);
            int cham2 = pop_lsb(chamBoard - 1 & chamBoard);

            int isDeath = m & move_kingc1mask;
            deathSq = pop_lsb(deathSquares[cham1][to][0]);
            position[notToPlay + coordinator] |= 1ULL * (isDeath > 0) << deathSq;
            position[notToPlay]               |= 1ULL * (isDeath > 0) << deathSq;
            pieceList[deathSq] += coordinator * (isDeath > 0);

            zobristHashUpdate ^= (isDeath != 0) * get_zobrist_hash(deathSq, coordinator, toPlay);

            isDeath = m & move_kingc2mask;
            deathSq = pop_lsb(deathSquares[cham1][to][1]);
            position[notToPlay + coordinator] |= 1ULL * (isDeath > 0) << deathSq;
            position[notToPlay]               |= 1ULL * (isDeath > 0) << deathSq;
            pieceList[deathSq] += coordinator * (isDeath > 0);

            zobristHashUpdate ^= (isDeath != 0) * get_zobrist_hash(deathSq, coordinator, toPlay);

            isDeath = m & move_kingc3mask;
            deathSq = pop_lsb(deathSquares[cham2][to][0]);
            position[notToPlay + coordinator] |= 1ULL * (isDeath > 0) << deathSq;
            position[notToPlay]               |= 1ULL * (isDeath > 0) << deathSq;
            pieceList[deathSq] += coordinator * (isDeath > 0);

            zobristHashUpdate ^= (isDeath != 0) * get_zobrist_hash(deathSq, coordinator, toPlay);

            isDeath = m & move_kingc4mask;
            deathSq = pop_lsb(deathSquares[cham2][to][1]);
            position[notToPlay + coordinator] |= 1ULL * (isDeath > 0) << deathSq;
            position[notToPlay]               |= 1ULL * (isDeath > 0) << deathSq;
            pieceList[deathSq] += coordinator * (isDeath > 0);

            zobristHashUpdate ^= (isDeath != 0) * get_zobrist_hash(deathSq, coordinator, toPlay);

            break;

        case springer:

            // just a garbage variable... coordinateSq in this case is where the springer captured
            // a piece at
            coordinateSq = pop_lsb(springerCaptures[from][to]);

            position[notToPlay + c1] |= 1ULL * (c1 > 0) << coordinateSq;
            position[notToPlay]      |= 1ULL * (c1 > 0) << coordinateSq;
            pieceList[coordinateSq] += c1;

            zobristHashUpdate ^= (c1 != 0) * get_zobrist_hash(coordinateSq, c1, toPlay);

            break;

        case retractor:

            // coordinateSq in this case is where the retractor captured a piece at
            coordinateSq = pop_lsb(retractorCaptures[from][to]);

            position[notToPlay + c1] |= 1ULL * (c1 > 0) << coordinateSq;
            position[notToPlay]      |= 1ULL * (c1 > 0) << coordinateSq;
            pieceList[coordinateSq] += c1;

            zobristHashUpdate ^= (c1 != 0) * get_zobrist_hash(coordinateSq, c1, toPlay);

            break;

        case chameleon:
            // straddler moves
            c1 = (m >> 15) & 1;
            c2 = (m >> 16) & 1;
            c3 = (m >> 17) & 1;
            c4 = (m >> 18) & 1;

            // up
            position[notToPlay + straddler] |= 1ULL * c1 << (to - 8);
            position[notToPlay]             |= 1ULL * c1 << (to - 8);
            pieceList[to - 8] += c1;

            zobristHashUpdate ^= c1 * get_zobrist_hash(to - 8, straddler, toPlay);

            // left
            position[notToPlay + straddler] |= 1ULL * c2 << (to - 1);
            position[notToPlay]             |= 1ULL * c2 << (to - 1);
            pieceList[to - 1] += c2;

            zobristHashUpdate ^= c2 * get_zobrist_hash(to - 1, straddler, toPlay);

            // right
            position[notToPlay + straddler] |= 1ULL * c3 << (to + 1);
            position[notToPlay]             |= 1ULL * c3 << (to + 1);
            pieceList[to + 1] += c3;

            zobristHashUpdate ^= c3 * get_zobrist_hash(to + 1, straddler, toPlay);

            // down
            position[notToPlay + straddler] |= 1ULL * c4 << (to + 8);
            position[notToPlay]             |= 1ULL * c4 << (to + 8);
            pieceList[to + 8] += c4;

            zobristHashUpdate ^= c4 * get_zobrist_hash(to + 8, straddler, toPlay);

            // consider coordinator moves
            coordinateSq = pop_lsb(position[toPlay + king]);
            c1 = (m >> 19) & 1;
            U64 death = deathSquares[to][coordinateSq][0];
            int captureSq = pop_lsb(death);
            position[notToPlay + coordinator]   |= death * c1;
            position[notToPlay]                 |= death * c1;
            pieceList[captureSq] += coordinator * c1;

            zobristHashUpdate ^= c1 * get_zobrist_hash(captureSq, coordinator, toPlay);

            // other death square for coordinator
            c1 = (m >> 20) & 1;
            death = deathSquares[to][coordinateSq][1];
            captureSq = pop_lsb(death);
            position[notToPlay + coordinator]   |= death * c1;
            position[notToPlay]                 |= death * c1;
            pieceList[captureSq] += coordinator * c1;

            zobristHashUpdate ^= c1 * get_zobrist_hash(captureSq, coordinator, toPlay);

            // consider retractor moves
            c1 = (m >> 21) & 1;
            int sq = pop_lsb(retractorCaptures[from][to]);
            position[notToPlay + retractor] |= 1ULL * c1 << sq;
            position[notToPlay]             |= 1ULL * c1 << sq;
            pieceList[sq] += retractor * c1;

            zobristHashUpdate ^= c1 * get_zobrist_hash(sq, retractor, toPlay);

            // consider springer moves
            c1 = (m >> 22) & 1;
            sq = pop_lsb(springerCaptures[from][to]);
            position[notToPlay + springer]  |= 1ULL * c1 << sq;
            position[notToPlay]             |= 1ULL * c1 << sq;
            pieceList[sq] += springer * c1;

            zobristHashUpdate ^= c1 * get_zobrist_hash(sq, springer, toPlay);

            break;
    }

    zobristHash ^= zobristHashUpdate;
}

void makeNullMove()
{
    // toggle turn on zobrist hash
    zobristHash ^= zobristHashes[ZOBRIST_HASH_COUNT - 1];

    // toggle turn
    toPlay = !toPlay * 8;
    notToPlay = !notToPlay * 8;
}
