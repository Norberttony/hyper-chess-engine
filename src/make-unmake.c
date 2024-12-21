
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

    // incrementally update evaluation parameters
    int pers = ((toPlay == white) << 1) - 1;
    materialScore += pers * (PSQT(type, toPlay, to) - PSQT(type, toPlay, from));

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
            unset_piece(notToPlay, c1, c1 > 0, upSq);

            // left
            unset_piece(notToPlay, c2, c2 > 0, ltSq);

            // right
            unset_piece(notToPlay, c3, c3 > 0, rtSq);

            // down
            unset_piece(notToPlay, c4, c4 > 0, dnSq);

            // perform incremental updates on the evaluation parameters
            // captured opponent's pieces, so I gain points
            materialScore += pers * (
                PSQT(c1, notToPlay, upSq * (upSq >= 0))
                + PSQT(c2, notToPlay, ltSq * (ltSq >= 0))
                + PSQT(c3, notToPlay, rtSq * (rtSq < 64))
                + PSQT(c4, notToPlay, dnSq * (dnSq < 64))
            );

            break;
        
        case immobilizer:
            break;

        case coordinator:
            coordinateSq = pop_lsb(position[toPlay + king]);
            
            // top death square
            int top = pop_lsb(deathSquares[coordinateSq][to][0]);
            unset_piece(notToPlay, c1, c1 > 0, top);

            // bottom death square
            int bottom = pop_lsb(deathSquares[coordinateSq][to][1]);
            unset_piece(notToPlay, c2, c2 > 0, bottom);

            // update zobrist hash by removing captured pieces
            zobristHashUpdate ^= 
                ((c1 != 0) * get_zobrist_hash(top, c1, toPlay)) ^
                ((c2 != 0) * get_zobrist_hash(bottom, c2, toPlay));

            // perform incremental updates on the evaluation parameters
            materialScore += pers * (
                PSQT(c1, notToPlay, top)
                + PSQT(c2, notToPlay, bottom)
            );

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
            int deathSqTop = pop_lsb(deathSquares[coordinateSq][to][0]);
            zobristHashUpdate ^= (c2 != 0) * get_zobrist_hash(deathSqTop, c2, toPlay);
            unset_piece(notToPlay, c2, c2 > 0, deathSqTop);

            // bottom death square
            int deathSqBottom = pop_lsb(deathSquares[coordinateSq][to][1]);
            zobristHashUpdate ^= (c3 != 0) * get_zobrist_hash(deathSqBottom, c3, toPlay);
            unset_piece(notToPlay, c3, c3 > 0, deathSqBottom);

            // the king moving can form death squares with chameleons but only against the coordinator
            U64 chamBoard = position[toPlay + chameleon];
            int cham1 = pop_lsb(chamBoard);
            U64 cham2Board = chamBoard & (chamBoard - 1);
            int cham2 = pop_lsb(chamBoard - 1 & chamBoard);

            // there's only one coordinator, so there can only be one square where it is captured.
            // since this is makeMove, the enemy coordinator is still on the board. so, we can at
            // most one bit turned on for this bitboard.
            U64 coordDeath = (
                (chamBoard > 0) * (
                    deathSquares[cham1][to][0] |
                    deathSquares[cham1][to][1]
                ) |
                (cham2Board > 0) * (
                    deathSquares[cham2][to][0] |
                    deathSquares[cham2][to][1]
                )
            ) & position[notToPlay + coordinator];

            int deathSqC = pop_lsb(coordDeath);
            int coordCapt = coordDeath > 0;
            
            unset_piece(notToPlay, coordinator, coordCapt, deathSqC);

            zobristHashUpdate ^= coordCapt * get_zobrist_hash(deathSqC, coordinator, toPlay);

            // perform incremental updates on the evaluation parameters
            materialScore += pers * (
                PSQT(c1, notToPlay, to)
                + PSQT(c2, notToPlay, deathSqTop)
                + PSQT(c3, notToPlay, deathSqBottom)
                + PSQT(coordCapt * coordinator, notToPlay, deathSqC)
            );

            break;

        case springer:

            // just a garbage variable... coordinateSq in this case is where the springer captured
            // a piece at
            coordinateSq = pop_lsb(springerCaptures[from][to]);

            unset_piece(notToPlay, c1, c1 > 0, coordinateSq);

            zobristHashUpdate ^= (c1 != 0) * get_zobrist_hash(coordinateSq, c1, toPlay);

            // perform incremental updates on the evaluation parameters
            materialScore += pers * PSQT(c1, notToPlay, coordinateSq);

            break;
        
        case retractor:

            // coordinateSq in this case is where the retractor captured a piece at
            coordinateSq = pop_lsb(retractorCaptures[from][to]);

            unset_piece(notToPlay, c1, c1 > 0, coordinateSq);

            zobristHashUpdate ^= (c1 != 0) * get_zobrist_hash(coordinateSq, c1, toPlay);

            // perform incremental updates on the evaluation parameters
            materialScore += pers * PSQT(c1, notToPlay, coordinateSq);

            break;

        case chameleon:
            // straddler moves
            c1 = (m >> 15) & 1;
            c2 = (m >> 16) & 1;
            c3 = (m >> 17) & 1;
            c4 = (m >> 18) & 1;

            int upSqc = to - 8;
            int ltSqc = to - 1;
            int rtSqc = to + 1;
            int dnSqc = to + 8;

            // up
            unset_piece(notToPlay, straddler, c1, upSqc);

            zobristHashUpdate ^= c1 * get_zobrist_hash(upSqc, c1, toPlay);

            // left
            unset_piece(notToPlay, straddler, c2, ltSqc);

            zobristHashUpdate ^= c2 * get_zobrist_hash(ltSqc, c2, toPlay);

            // right
            unset_piece(notToPlay, straddler, c3, rtSqc);

            zobristHashUpdate ^= c3 * get_zobrist_hash(rtSqc, c3, toPlay);

            // down
            unset_piece(notToPlay, straddler, c4, dnSqc);

            zobristHashUpdate ^= c4 * get_zobrist_hash(dnSqc, c4, toPlay);

            // chameleon might try to coordinate with the king...
            coordinateSq = pop_lsb(position[toPlay + king]);

            // consider coordinator moves
            int c5 = (m >> 19) & 1;
            U64 death = deathSquares[to][coordinateSq][0];
            int coordCaptSqTop = pop_lsb(death);
            unset_piece(notToPlay, coordinator, c5, coordCaptSqTop);

            zobristHashUpdate ^= c5 * get_zobrist_hash(coordCaptSqTop, coordinator, toPlay);

            // other death square for coordinator
            int c6 = (m >> 20) & 1;
            death = deathSquares[to][coordinateSq][1];
            int coordCaptSqBottom = pop_lsb(death);
            unset_piece(notToPlay, coordinator, c6, coordCaptSqBottom);

            zobristHashUpdate ^= c6 * get_zobrist_hash(coordCaptSqBottom, coordinator, toPlay);

            // consider retractor moves
            int c7 = (m >> 21) & 1;
            int retrCaptSq = pop_lsb(retractorCaptures[from][to]);
            unset_piece(notToPlay, retractor, c7, retrCaptSq);

            zobristHashUpdate ^= c7 * get_zobrist_hash(retrCaptSq, retractor, toPlay);

            // consider springer moves
            int c8 = (m >> 22) & 1;
            int spriCaptSq = pop_lsb(springerCaptures[from][to]);
            unset_piece(notToPlay, springer, c8, spriCaptSq);

            zobristHashUpdate ^= c8 * get_zobrist_hash(spriCaptSq, springer, toPlay);

            // perform incremental updates on the evaluation parameters
            int t1 = c1 * straddler;
            int t2 = c2 * straddler;
            int t3 = c3 * straddler;
            int t4 = c4 * straddler;
            int t56 = (c5 + c6) * coordinator;
            int t7 = c7 * retractor;
            int t8 = c8 * springer;

            materialScore += pers * (
                PSQT(t1, notToPlay, upSqc * (upSqc >= 0))
                + PSQT(t2, notToPlay, ltSqc * (ltSqc >= 0))
                + PSQT(t3, notToPlay, rtSqc * (rtSqc < 64))
                + PSQT(t4, notToPlay, dnSqc * (dnSqc < 64))
                + PSQT(t56, notToPlay, c5 * coordCaptSqTop + c6 * coordCaptSqBottom)
                + PSQT(t7, notToPlay, retrCaptSq)
                + PSQT(t8, notToPlay, spriCaptSq)
            );

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

    // incrementally update evaluation parameters
    int pers = ((toPlay == white) << 1) - 1;
    materialScore += pers * (PSQT(type, toPlay, from) - PSQT(type, toPlay, to));

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
            set_piece(notToPlay, c1, c1 > 0, upSq);

            // left
            set_piece(notToPlay, c2, c2 > 0, ltSq);

            // right
            set_piece(notToPlay, c3, c3 > 0, rtSq);

            // down
            set_piece(notToPlay, c4, c4 > 0, dnSq);

            // perform incremental updates on the evaluation parameters
            // captured opponent's pieces, so I lose points (place back captured pieces)
            materialScore -= pers * (
                PSQT(c1, notToPlay, upSq * (upSq >= 0))
                + PSQT(c2, notToPlay, ltSq * (ltSq >= 0))
                + PSQT(c3, notToPlay, rtSq * (rtSq < 64))
                + PSQT(c4, notToPlay, dnSq * (dnSq < 64))
            );

            break;

        case immobilizer:
            break;
        
        case coordinator:
            coordinateSq = pop_lsb(position[toPlay + king]);
            
            // top death square
            int top = pop_lsb(deathSquares[coordinateSq][to][0]);
            set_piece(notToPlay, c1, c1 > 0, top);

            // bottom death square
            int bottom = pop_lsb(deathSquares[coordinateSq][to][1]);
            set_piece(notToPlay, c2, c2 > 0, bottom);

            zobristHashUpdate ^=
                ((c1 != 0) * get_zobrist_hash(top, c1, toPlay)) ^
                ((c2 != 0) * get_zobrist_hash(bottom, c2, toPlay));

            // perform incremental updates on the evaluation parameters
            materialScore -= pers * (
                PSQT(c1, notToPlay, top)
                + PSQT(c2, notToPlay, bottom)
            );

            break;

        case king:

            // capture by displacement
            set_piece(notToPlay, c1, c1 > 0, to);

            // uncapture piece on zobrist hash
            zobristHashUpdate ^= (c1 != 0) * get_zobrist_hash(to, c1, toPlay);

            // death squares with coordinator
            // can form death squares with own coordinator
            coordinateSq = pop_lsb(position[toPlay + coordinator]);

            int deathSqTop = pop_lsb(deathSquares[coordinateSq][to][0]);
            set_piece(notToPlay, c2, c2 > 0, deathSqTop);
            
            zobristHashUpdate ^= (c2 != 0) * get_zobrist_hash(deathSqTop, c2, toPlay);

            int deathSqBottom = pop_lsb(deathSquares[coordinateSq][to][1]);
            set_piece(notToPlay, c3, c3 > 0, deathSqBottom);

            zobristHashUpdate ^= (c3 != 0) * get_zobrist_hash(deathSqBottom, c3, toPlay);

            // consider king-chameleon duo
            U64 chamBoard = position[toPlay + chameleon];
            int cham1 = pop_lsb(chamBoard);
            int cham2 = pop_lsb(chamBoard - 1 & chamBoard);

            int isDeath1 = get_kb_c1(m);
            int isDeath2 = get_kb_c2(m);
            int isDeath3 = get_kb_c3(m);
            int isDeath4 = get_kb_c4(m);

            U64 coordDeath =
                isDeath1 * deathSquares[cham1][to][0] |
                isDeath2 * deathSquares[cham1][to][1] |
                isDeath3 * deathSquares[cham2][to][0] |
                isDeath4 * deathSquares[cham2][to][1];
            
            int deathSqC = pop_lsb(coordDeath);
            int coordCapt = coordDeath > 0;

            set_piece(notToPlay, coordinator, coordCapt, deathSqC);

            zobristHashUpdate ^= coordCapt * get_zobrist_hash(deathSqC, coordinator, toPlay);

            // perform incremental updates on the evaluation parameters
            materialScore -= pers * (
                PSQT(c1, notToPlay, to)
                + PSQT(c2, notToPlay, deathSqTop)
                + PSQT(c3, notToPlay, deathSqBottom)
                + PSQT(coordCapt * coordinator, notToPlay, deathSqC)
            );

            break;

        case springer:

            // just a garbage variable... coordinateSq in this case is where the springer captured
            // a piece at
            coordinateSq = pop_lsb(springerCaptures[from][to]);

            set_piece(notToPlay, c1, c1 > 0, coordinateSq);

            zobristHashUpdate ^= (c1 != 0) * get_zobrist_hash(coordinateSq, c1, toPlay);

            // perform incremental updates on the evaluation parameters
            materialScore -= pers * PSQT(c1, notToPlay, coordinateSq);

            break;

        case retractor:

            // coordinateSq in this case is where the retractor captured a piece at
            coordinateSq = pop_lsb(retractorCaptures[from][to]);

            set_piece(notToPlay, c1, c1 > 0, coordinateSq);

            zobristHashUpdate ^= (c1 != 0) * get_zobrist_hash(coordinateSq, c1, toPlay);

            // perform incremental updates on the evaluation parameters
            materialScore -= pers * PSQT(c1, notToPlay, coordinateSq);

            break;

        case chameleon:
            // straddler moves
            c1 = (m >> 15) & 1;
            c2 = (m >> 16) & 1;
            c3 = (m >> 17) & 1;
            c4 = (m >> 18) & 1;

            int upSqc = to - 8;
            int ltSqc = to - 1;
            int rtSqc = to + 1;
            int dnSqc = to + 8;

            // up
            set_piece(notToPlay, straddler, c1, upSqc);
            zobristHashUpdate ^= c1 * get_zobrist_hash(upSqc, straddler, toPlay);

            // left
            set_piece(notToPlay, straddler, c2, ltSqc);
            zobristHashUpdate ^= c2 * get_zobrist_hash(ltSqc, straddler, toPlay);

            // right
            set_piece(notToPlay, straddler, c3, rtSqc);
            zobristHashUpdate ^= c3 * get_zobrist_hash(rtSqc, straddler, toPlay);

            // down
            set_piece(notToPlay, straddler, c4, dnSqc);
            zobristHashUpdate ^= c4 * get_zobrist_hash(dnSqc, straddler, toPlay);

            // chameleon might try to coordinate with the king...
            coordinateSq = pop_lsb(position[toPlay + king]);

            // consider coordinator moves
            int c5 = (m >> 19) & 1;
            U64 death = deathSquares[to][coordinateSq][0];
            int coordCaptSqTop = pop_lsb(death);

            set_piece(notToPlay, coordinator, c5, coordCaptSqTop);
            zobristHashUpdate ^= c5 * get_zobrist_hash(coordCaptSqTop, coordinator, toPlay);

            // other death square for coordinator
            int c6 = (m >> 20) & 1;
            death = deathSquares[to][coordinateSq][1];
            int coordCaptSqBottom = pop_lsb(death);

            set_piece(notToPlay, coordinator, c6, coordCaptSqBottom);
            zobristHashUpdate ^= c6 * get_zobrist_hash(coordCaptSqBottom, coordinator, toPlay);

            // consider retractor moves
            int c7 = (m >> 21) & 1;
            int retrCaptSq = pop_lsb(retractorCaptures[from][to]);

            set_piece(notToPlay, retractor, c7, retrCaptSq);
            zobristHashUpdate ^= c7 * get_zobrist_hash(retrCaptSq, retractor, toPlay);

            // consider springer moves
            int c8 = (m >> 22) & 1;
            int spriCaptSq = pop_lsb(springerCaptures[from][to]);

            set_piece(notToPlay, springer, c8, spriCaptSq);
            zobristHashUpdate ^= c8 * get_zobrist_hash(spriCaptSq, springer, toPlay);

            // perform incremental updates on the evaluation parameters
            int t1 = c1 * straddler;
            int t2 = c2 * straddler;
            int t3 = c3 * straddler;
            int t4 = c4 * straddler;
            int t56 = (c5 + c6) * coordinator;
            int t7 = c7 * retractor;
            int t8 = c8 * springer;

            materialScore -= pers * (
                PSQT(t1, notToPlay, upSqc * (upSqc >= 0))
                + PSQT(t2, notToPlay, ltSqc * (ltSqc >= 0))
                + PSQT(t3, notToPlay, rtSqc * (rtSqc < 64))
                + PSQT(t4, notToPlay, dnSqc * (dnSqc < 64))
                + PSQT(t56, notToPlay, c5 * coordCaptSqTop + c6 * coordCaptSqBottom)
                + PSQT(t7, notToPlay, retrCaptSq)
                + PSQT(t8, notToPlay, spriCaptSq)
            );

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
