
#include "make-unmake.h"

// this function assumes that the piece exists
static inline __attribute__((always_inline)) void setPiece(int stp, int type, int sq)
{
    U64 sqBoard = 1ULL << sq;
    g_pos.boards[stp + type] |= sqBoard;
    g_pos.boards[stp]        |= sqBoard;
    g_pos.pieceList[sq] = type;
    g_pos.materialScore[stp == black] += PSQT(type, stp, sq);
}

static inline __attribute__((always_inline)) void unsetPiece(int stp, int type, int sq)
{
    U64 sqBoard = 1ULL << sq;
    g_pos.boards[stp + type] ^= sqBoard;
    g_pos.boards[stp] ^= sqBoard;
    g_pos.pieceList[sq] = 0;
    g_pos.materialScore[stp == black] -= PSQT(type, stp, sq);
}


void makeMove(Move m)
{
    // decode move
    int type = get_type(m);
    int from = get_from(m);
    int to = get_to(m);

    int c1 = get_c1(m);
    int c2 = get_c2(m);
    int c3 = get_c3(m);
    int c4 = get_c4(m);

    int toPlay = g_pos.toPlay;
    int notToPlay = g_pos.notToPlay;

    int coordinateSq;

    // incrementally update evaluation parameters
    g_pos.materialScore[toPlay == black] += PSQT(type, toPlay, to) - PSQT(type, toPlay, from);

    U64 zobristHashUpdate = 0ULL;

    // interpret capture bits
    switch(type)
    {
        case straddler:

            // up
            if (__builtin_expect(c1, 0))
            {
                unsetPiece(notToPlay, c1, to - 8);
                zobristHashUpdate ^= get_zobrist_hash(to - 8, c1, toPlay);
            }

            // left
            if (__builtin_expect(c2, 0))
            {
                unsetPiece(notToPlay, c2, to - 1);
                zobristHashUpdate ^= get_zobrist_hash(to - 1, c2, toPlay);
            }

            // right
            if (__builtin_expect(c3, 0))
            {
                unsetPiece(notToPlay, c3, to + 1);
                zobristHashUpdate ^= get_zobrist_hash(to + 1, c3, toPlay);
            }

            // down
            if (__builtin_expect(c4, 0))
            {
                unsetPiece(notToPlay, c4, to + 8);
                zobristHashUpdate ^= get_zobrist_hash(to + 8, c4, toPlay);
            }

            break;
        
        case immobilizer:
            break;

        case coordinator:
            coordinateSq = pop_lsb(g_pos.boards[toPlay + king]);
            
            // top death square
            if (__builtin_expect(c1, 0))
            {
                int top = pop_lsb(deathSquares[coordinateSq][to][0]);
                unsetPiece(notToPlay, c1, top);
                zobristHashUpdate ^= get_zobrist_hash(top, c1, toPlay);
            }

            // bottom death square
            if (__builtin_expect(c2, 0))
            {
                int bottom = pop_lsb(deathSquares[coordinateSq][to][1]);
                unsetPiece(notToPlay, c2, bottom);
                zobristHashUpdate ^= get_zobrist_hash(bottom, c2, toPlay);
            }

            break;

        case king:

            // can capture by displacement
            // assumes pieceList will be updated by king overwriting square
            if (__builtin_expect(c1, 0))
            {
                g_pos.boards[notToPlay + c1] ^= 1ULL << to;
                g_pos.boards[notToPlay]      ^= 1ULL << to;
                g_pos.materialScore[notToPlay == black] -= PSQT(c1, notToPlay, to);
                zobristHashUpdate ^= get_zobrist_hash(to, c1, toPlay);
            }

            // can form death squares with own coordinator
            coordinateSq = pop_lsb(g_pos.boards[toPlay + coordinator]);

            // top death square
            if (__builtin_expect(c2, 0))
            {
                int top = pop_lsb(deathSquares[coordinateSq][to][0]);
                unsetPiece(notToPlay, c2, top);
                zobristHashUpdate ^= get_zobrist_hash(top, c2, toPlay);
            }

            // bottom death square
            if (__builtin_expect(c3, 0))
            {
                int bottom = pop_lsb(deathSquares[coordinateSq][to][1]);
                unsetPiece(notToPlay, c3, bottom);
                zobristHashUpdate ^= get_zobrist_hash(bottom, c3, toPlay);
            }

            // there's only one coordinator, so there can only be one square where it is captured.
            // since this is makeMove, the enemy coordinator is still on the board. so, we can have
            // at most one bit turned on for this bitboard.
            if (__builtin_expect(get_kb_c(m), 0))
            {
                int deathSqC = pop_lsb(g_pos.boards[notToPlay + coordinator]);
                unsetPiece(notToPlay, coordinator, deathSqC);
                zobristHashUpdate ^= get_zobrist_hash(deathSqC, coordinator, toPlay);
            }

            break;

        case springer:

            if (__builtin_expect(c1, 0))
            {
                int captSq = pop_lsb(springerCaptures[from][to]);
                unsetPiece(notToPlay, c1, captSq);
                zobristHashUpdate ^= get_zobrist_hash(captSq, c1, toPlay);
            }

            break;
        
        case retractor:

            if (__builtin_expect(c1, 0))
            {
                int captSq = pop_lsb(retractorCaptures[from][to]);
                unsetPiece(notToPlay, c1, captSq);
                zobristHashUpdate ^= get_zobrist_hash(captSq, c1, toPlay);
            }

            break;

        case chameleon:
            // straddler moves
            c1 = get_b_cu(m);
            c2 = get_b_cl(m);
            c3 = get_b_cr(m);
            c4 = get_b_cd(m);

            // up
            if (__builtin_expect(c1, 0))
            {
                unsetPiece(notToPlay, straddler, to - 8);
                zobristHashUpdate ^= get_zobrist_hash(to - 8, straddler, toPlay);
            }

            // left
            if (__builtin_expect(c2, 0))
            {
                unsetPiece(notToPlay, straddler, to - 1);
                zobristHashUpdate ^= get_zobrist_hash(to - 1, straddler, toPlay);
            }

            // right
            if (__builtin_expect(c3, 0))
            {
                unsetPiece(notToPlay, straddler, to + 1);
                zobristHashUpdate ^= get_zobrist_hash(to + 1, straddler, toPlay);
            }

            // down
            if (__builtin_expect(c4, 0))
            {
                unsetPiece(notToPlay, straddler, to + 8);
                zobristHashUpdate ^= get_zobrist_hash(to + 8, straddler, toPlay);
            }

            // chameleon might try to coordinate with the king...
            coordinateSq = pop_lsb(g_pos.boards[toPlay + king]);

            // consider coordinator moves
            if (__builtin_expect(get_b_cd1(m), 0))
            {
                U64 death = deathSquares[to][coordinateSq][0];
                int top = pop_lsb(death);
                unsetPiece(notToPlay, coordinator, top);
                zobristHashUpdate ^= get_zobrist_hash(top, coordinator, toPlay);
            }

            // other death square for coordinator
            if (__builtin_expect(get_b_cd2(m), 0))
            {
                U64 death = deathSquares[to][coordinateSq][1];
                int bottom = pop_lsb(death);
                unsetPiece(notToPlay, coordinator, bottom);
                zobristHashUpdate ^= get_zobrist_hash(bottom, coordinator, toPlay);
            }

            // consider retractor moves
            if (__builtin_expect(get_b_cq(m), 0))
            {
                int captSq = pop_lsb(retractorCaptures[from][to]);
                unsetPiece(notToPlay, retractor, captSq);
                zobristHashUpdate ^= get_zobrist_hash(captSq, retractor, toPlay);
            }

            // consider springer moves
            if (__builtin_expect(get_b_cn(m), 0))
            {
                int captSq = pop_lsb(springerCaptures[from][to]);
                unsetPiece(notToPlay, springer, captSq);
                zobristHashUpdate ^= get_zobrist_hash(captSq, springer, toPlay);
            }

            break;
    }

    // move piece
    U64 toggle = (1ULL << from) | (1ULL << to);
    g_pos.boards[toPlay + type] ^= toggle;
    g_pos.boards[toPlay] ^= toggle;

    // remove piece from zobrist hash
    zobristHashUpdate ^= get_zobrist_hash(from, type, !toPlay);
    // add piece back
    zobristHashUpdate ^= get_zobrist_hash(to, type, !toPlay);

    // update piece list
    g_pos.pieceList[to] = g_pos.pieceList[from];
    g_pos.pieceList[from] = 0;

    // toggle turn
    g_pos.toPlay = !toPlay * 8;
    g_pos.notToPlay = !notToPlay * 8;

    // toggle turn on zobrist hash
    zobristHashUpdate ^= zobristHashes[ZOBRIST_HASH_COUNT - 1];

    // apply zobrist hash updates
    g_pos.zobristHash ^= zobristHashUpdate;

    // add to repeat table
    repeatTable[repeatTableIndex++] = g_pos.zobristHash;
    repeatTableIndex -= REPEAT_TABLE_ENTRIES * (repeatTableIndex == REPEAT_TABLE_ENTRIES);

    // update halfmove counter
    g_pos.halfmove++;
}

void unmakeMove(Move m)
{
    // decode move
    int type = get_type(m);
    int from = get_from(m);
    int to = get_to(m);

    int c1 = get_c1(m);
    int c2 = get_c2(m);
    int c3 = get_c3(m);
    int c4 = get_c4(m);

    int coordinateSq;
    U64 zobristHashUpdate = 0ULL;

    // update halfmove counter
    g_pos.halfmove--;

    // remove from repeat table
    repeatTableIndex += REPEAT_TABLE_ENTRIES * (repeatTableIndex == 0);
    repeatTable[--repeatTableIndex] = 0ULL;

    // toggle turn on zobrist hash
    zobristHashUpdate ^= zobristHashes[ZOBRIST_HASH_COUNT - 1];

    // toggle turn
    g_pos.toPlay = !g_pos.toPlay * 8;
    g_pos.notToPlay = !g_pos.notToPlay * 8;

    int toPlay = g_pos.toPlay;
    int notToPlay = g_pos.notToPlay;

    // remove piece from zobrist hash
    zobristHashUpdate ^= get_zobrist_hash(to, type, !toPlay);
    // add piece back
    zobristHashUpdate ^= get_zobrist_hash(from, type, !toPlay);

    // unmove piece
    U64 toggle = (1ULL << from) | (1ULL << to);
    g_pos.boards[toPlay + type] ^= toggle;
    g_pos.boards[toPlay] ^= toggle;

    // update piece list
    g_pos.pieceList[from] = g_pos.pieceList[to];
    g_pos.pieceList[to] = 0;

    // incrementally update evaluation parameters
    g_pos.materialScore[toPlay == black] += PSQT(type, toPlay, from) - PSQT(type, toPlay, to);

    // interpret capture bits
    switch(type)
    {
        case straddler:

            // up
            if (__builtin_expect(c1, 0))
            {
                setPiece(notToPlay, c1, to - 8);
                zobristHashUpdate ^= get_zobrist_hash(to - 8, c1, toPlay);
            }

            // left
            if (__builtin_expect(c2, 0))
            {
                setPiece(notToPlay, c2, to - 1);
                zobristHashUpdate ^= get_zobrist_hash(to - 1, c2, toPlay);
            }

            // right
            if (__builtin_expect(c3, 0))
            {
                setPiece(notToPlay, c3, to + 1);
                zobristHashUpdate ^= get_zobrist_hash(to + 1, c3, toPlay);
            }

            // down
            if (__builtin_expect(c4, 0))
            {
                setPiece(notToPlay, c4, to + 8);
                zobristHashUpdate ^= get_zobrist_hash(to + 8, c4, toPlay);
            }

            break;

        case immobilizer:
            break;
        
        case coordinator:
            coordinateSq = pop_lsb(g_pos.boards[toPlay + king]);
            
            // top death square
            if (__builtin_expect(c1, 0))
            {
                int top = pop_lsb(deathSquares[coordinateSq][to][0]);
                setPiece(notToPlay, c1, top);
                zobristHashUpdate ^= get_zobrist_hash(top, c1, toPlay);
            }

            // bottom death square
            if (__builtin_expect(c2, 0))
            {
                int bottom = pop_lsb(deathSquares[coordinateSq][to][1]);
                setPiece(notToPlay, c2, bottom);
                zobristHashUpdate ^= get_zobrist_hash(bottom, c2, toPlay);
            }

            break;

        case king:

            // capture by displacement
            if (__builtin_expect(c1, 0))
            {
                setPiece(notToPlay, c1, to);
                zobristHashUpdate ^= get_zobrist_hash(to, c1, toPlay);
            }

            // death squares with coordinator
            // can form death squares with own coordinator
            coordinateSq = pop_lsb(g_pos.boards[toPlay + coordinator]);

            if (__builtin_expect(c2, 0))
            {
                int top = pop_lsb(deathSquares[coordinateSq][to][0]);
                setPiece(notToPlay, c2, top);
                zobristHashUpdate ^= get_zobrist_hash(top, c2, toPlay);
            }

            if (__builtin_expect(c3, 0))
            {
                int bottom = pop_lsb(deathSquares[coordinateSq][to][1]);
                setPiece(notToPlay, c3, bottom);
                zobristHashUpdate ^= get_zobrist_hash(bottom, c3, toPlay);
            }

            // consider king-chameleon duo
            if (__builtin_expect(get_kb_c(m), 0))
            {
                U64 chamBoard = g_pos.boards[toPlay + chameleon];
                int cham1 = pop_lsb(chamBoard);
                int cham2 = pop_lsb((chamBoard - 1) & chamBoard);

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
                setPiece(notToPlay, coordinator, deathSqC);
                zobristHashUpdate ^= get_zobrist_hash(deathSqC, coordinator, toPlay);
            }

            break;

        case springer:

            if (__builtin_expect(c1, 0))
            {
                int captSq = pop_lsb(springerCaptures[from][to]);
                setPiece(notToPlay, c1, captSq);
                zobristHashUpdate ^= get_zobrist_hash(captSq, c1, toPlay);
            }

            break;

        case retractor:

            // coordinateSq in this case is where the retractor captured a piece at

            if (__builtin_expect(c1, 0))
            {
                int captSq = pop_lsb(retractorCaptures[from][to]);
                setPiece(notToPlay, c1, captSq);
                zobristHashUpdate ^= get_zobrist_hash(captSq, c1, toPlay);
            }

            break;

        case chameleon:
            // straddler moves
            c1 = get_b_cu(m);
            c2 = get_b_cl(m);
            c3 = get_b_cr(m);
            c4 = get_b_cd(m);

            // up
            if (__builtin_expect(c1, 0))
            {
                setPiece(notToPlay, straddler, to - 8);
                zobristHashUpdate ^= get_zobrist_hash(to - 8, straddler, toPlay);
            }

            // left
            if (__builtin_expect(c2, 0))
            {
                setPiece(notToPlay, straddler, to - 1);
                zobristHashUpdate ^= get_zobrist_hash(to - 1, straddler, toPlay);
            }

            // right
            if (__builtin_expect(c3, 0))
            {
                setPiece(notToPlay, straddler, to + 1);
                zobristHashUpdate ^= get_zobrist_hash(to + 1, straddler, toPlay);
            }

            // down
            if (__builtin_expect(c4, 0))
            {
                setPiece(notToPlay, straddler, to + 8);
                zobristHashUpdate ^= get_zobrist_hash(to + 8, straddler, toPlay);
            }

            // chameleon might try to coordinate with the king...
            coordinateSq = pop_lsb(g_pos.boards[toPlay + king]);

            // consider coordinator captures
            if (__builtin_expect(get_b_cd1(m), 0))
            {
                U64 death = deathSquares[to][coordinateSq][0];
                int top = pop_lsb(death);
                setPiece(notToPlay, coordinator, top);
                zobristHashUpdate ^= get_zobrist_hash(top, coordinator, toPlay);
            }

            // other death square for coordinator
            if (__builtin_expect(get_b_cd2(m), 0))
            {
                U64 death = deathSquares[to][coordinateSq][1];
                int bottom = pop_lsb(death);
                setPiece(notToPlay, coordinator, bottom);
                zobristHashUpdate ^= get_zobrist_hash(bottom, coordinator, toPlay);
            }

            // consider retractor captures
            if (__builtin_expect(get_b_cq(m), 0))
            {
                int captSq = pop_lsb(retractorCaptures[from][to]);
                setPiece(notToPlay, retractor, captSq);
                zobristHashUpdate ^= get_zobrist_hash(captSq, retractor, toPlay);
            }

            // consider springer captures
            if (__builtin_expect(get_b_cn(m), 0))
            {
                int captSq = pop_lsb(springerCaptures[from][to]);
                setPiece(notToPlay, springer, captSq);
                zobristHashUpdate ^= get_zobrist_hash(captSq, springer, toPlay);
            }

            break;
    }

    g_pos.zobristHash ^= zobristHashUpdate;
}

void makeNullMove(void)
{
    // toggle turn on zobrist hash
    g_pos.zobristHash ^= zobristHashes[ZOBRIST_HASH_COUNT - 1];

    // toggle turn
    g_pos.toPlay = !g_pos.toPlay * 8;
    g_pos.notToPlay = !g_pos.notToPlay * 8;
}
