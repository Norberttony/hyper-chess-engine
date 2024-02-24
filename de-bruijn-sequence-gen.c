// generates debruijn numbers

#include <stdio.h>
#include <math.h>

// the order of the debruijn sequence. it is assumed that an alphabet of {0, 1} is used every time.
// ORDER should not exceed 6
#define ORDER 6
#define TWO_POW_ORDER (1 << ORDER)

struct Node
{
    int visited;
    struct Node *add0;
    struct Node *add1;
};

struct Node nodes[TWO_POW_ORDER];

const int bitmask = TWO_POW_ORDER - 1;
unsigned long long hasFound = 0; // only one debruijn sequence is necessary.

void search(struct Node*);
void search_helper(struct Node*, unsigned long long seq, int);

int main()
{

    // create a graph
    for (int i = 0; i < TWO_POW_ORDER; i++)
    {
        int shiftedIndex = (i << 1) & bitmask;
        nodes[i].add0 = &nodes[shiftedIndex];
        nodes[i].add1 = &nodes[shiftedIndex + 1];
    }

    printf("Displaying one valid debruijn sequences given B(2, %d)\n", ORDER);

    // try searching at every node
    for (int i = 0; i < TWO_POW_ORDER; i++)
    {
        search(&nodes[i]);

        // clear nodes for next traversal
        for (int j = 0; j < TWO_POW_ORDER; j++)
        {
            nodes[j].visited = 0;
        }
    }
    
    return 0;
}

struct Node* startNode = 0;

void search(struct Node* node)
{
    startNode = node;
    search_helper(node, 0, 0);
}

void search_helper(struct Node* node, unsigned long long seq, int numVisited)
{
    if (hasFound)
    {
        return;
    }

    // base case
    if (node->visited)
    {
        // must end up where it started, and must have visited each node
        if (node == startNode && numVisited == TWO_POW_ORDER)
        {
            hasFound = seq;

            // scan entire sequence to print it out
            unsigned long long i = 1ULL << 63;
            while (i)
            {
                printf("%d", (seq & i) > 0);
                i >>= 1;
            }
            printf("\n");
        }


        return;
    }

    // just search this node
    node->visited = 1;
    numVisited++;

    // search next two nodes
    search_helper(node->add0, seq << 1, numVisited);

    // as a special case for this program, only search zeroes for the first 6 iterations
    if (numVisited > 6)
    {
        search_helper(node->add1, (seq << 1) + 1, numVisited);
    }

    // unvisit node (backtracking)
    node->visited = 0;
}
