#include <stdio.h>
#include <string.h>

#define MAX 10
#define INF 999

int n;
char name[MAX][20];

/* Original network topology */
int originalCost[MAX][MAX];

/* Distance Vector data */
int dvCost[MAX][MAX];
int dvNextHop[MAX][MAX];

/* Link State data */
int lsDist[MAX][MAX];
int lsNextHop[MAX][MAX];

void readInput()
{
    printf("Enter number of nodes: ");
    scanf("%d", &n);

    printf("Enter node names:\n");
    for (int i = 0; i < n; i++)
        scanf("%s", name[i]);

    printf("\nEnter cost matrix row by row (-1 for no connection):\n");

    /* Tight column headers */
    printf("%-3s", "");
    for (int j = 0; j < n; j++)
    {
        printf(" %s", name[j]);
    }
    printf("\n");

    /* Read row by row with matching tight formatting */
    for (int i = 0; i < n; i++)
    {
        printf("%-3s", name[i]);
        for (int j = 0; j < n; j++)
        {
            int inputValue;
            scanf("%d", &inputValue);

            if (i == j)
            {
                originalCost[i][j] = 0;
            }
            else if (inputValue == -1)
            {
                originalCost[i][j] = INF;
            }
            else
            {
                originalCost[i][j] = inputValue;
            }
        }
    }
}

int findNodeIndex(char searchName[])
{
    for (int i = 0; i < n; i++)
    {
        if (strcmp(name[i], searchName) == 0)
            return i;
    }
    return -1;
}

void displayInitialTable()
{
    char searchName[20];
    printf("\nEnter node name to view its INITIAL routing table: ");
    scanf("%s", searchName);

    int i = findNodeIndex(searchName);
    if (i == -1)
    {
        printf("Node not found!\n");
        return;
    }

    printf("\nINITIAL ROUTING TABLE for Router %s\n", name[i]);
    printf("%-15s %-10s %-10s\n", "Destination", "Cost", "NextHop");
    printf("---------------------------------------------\n");
    for (int j = 0; j < n; j++)
    {
        if (i == j)
            continue;
        printf("%-15s", name[j]);
        if (originalCost[i][j] >= INF)
        {
            printf("%-10s %-10s\n", "INF", "-");
        }
        else
        {
            printf("%-10d %-10s\n", originalCost[i][j], name[j]);
        }
    }
}

void initDistanceVector()
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            dvCost[i][j] = originalCost[i][j];
            if (i == j)
                dvNextHop[i][j] = i;
            else if (originalCost[i][j] < INF)
                dvNextHop[i][j] = j;
            else
                dvNextHop[i][j] = -1;
        }
    }
}

void runDistanceVector()
{
    int updated = 1;
    while (updated)
    {
        updated = 0;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                if (i == j || originalCost[i][j] >= INF)
                    continue;
                for (int k = 0; k < n; k++)
                {
                    if (i == k)
                        continue;
                    if (dvCost[j][k] < INF &&
                        originalCost[i][j] + dvCost[j][k] < dvCost[i][k])
                    {
                        dvCost[i][k] = originalCost[i][j] + dvCost[j][k];
                        dvNextHop[i][k] = j;
                        updated = 1;
                    }
                }
            }
        }
    }
}

/* Helper to reconstruct and print the path string using the next hop array */
void printPath(int src, int dest, int nextHopArray[MAX][MAX])
{
    if (nextHopArray[src][dest] == -1)
    {
        printf("-");
        return;
    }

    printf("%s", name[src]);
    int current = src;
    while (current != dest)
    {
        current = nextHopArray[current][dest];
        if (current == -1) break;
        printf(" -> %s", name[current]);
    }
}

void printDVTableForNode(int i)
{
    printf("\nDISTANCE VECTOR ROUTING TABLE for Router %s\n", name[i]);
    printf("%-15s %-10s %-10s %-20s\n", "Destination", "Cost", "NextHop", "Shortest Path");
    printf("----------------------------------------------------------------------\n");
    for (int j = 0; j < n; j++)
    {
        if (i == j)
            continue;
        printf("%-15s", name[j]);
        if (dvCost[i][j] >= INF)
        {
            printf("%-10s %-10s %-20s\n", "INF", "-", "-");
        }
        else
        {
            printf("%-10d %-10s ", dvCost[i][j], name[dvNextHop[i][j]]);
            printPath(i, j, dvNextHop);
            printf("\n");
        }
    }
}

void displayDistanceVectorMenu()
{
    int option;
    printf("\nDistance Vector Views:\n1. View single node\n2. View all nodes at once\nEnter choice: ");
    scanf("%d", &option);

    if (option == 1)
    {
        char searchName[20];
        printf("Enter node name: ");
        scanf("%s", searchName);
        int i = findNodeIndex(searchName);
        if (i == -1) printf("Node not found!\n");
        else printDVTableForNode(i);
    }
    else if (option == 2)
    {
        for (int i = 0; i < n; i++)
            printDVTableForNode(i);
    }
    else
    {
        printf("Invalid View Option.\n");
    }
}

void dijkstra(int source)
{
    int visited[MAX] = {0};

    for (int i = 0; i < n; i++)
    {
        lsDist[source][i] = originalCost[source][i];
        if (i != source && originalCost[source][i] < INF)
        {
            lsNextHop[source][i] = i;
        }
        else
        {
            lsNextHop[source][i] = -1;
        }
    }

    lsDist[source][source] = 0;
    lsNextHop[source][source] = source;
    visited[source] = 1;

    for (int count = 1; count < n; count++)
    {
        int u = -1;
        int minDist = INF;
        for (int i = 0; i < n; i++)
        {
            if (!visited[i] && lsDist[source][i] < minDist)
            {
                minDist = lsDist[source][i];
                u = i;
            }
        }
        if (u == -1)
            break;
        visited[u] = 1;

        for (int v = 0; v < n; v++)
        {
            if (!visited[v] && originalCost[u][v] < INF)
            {
                if (lsDist[source][u] + originalCost[u][v] < lsDist[source][v])
                {
                    lsDist[source][v] = lsDist[source][u] + originalCost[u][v];
                    lsNextHop[source][v] = (u == source) ? v : lsNextHop[source][u];
                }
            }
        }
    }
}

void runLinkState()
{
    for (int i = 0; i < n; i++)
        dijkstra(i);
}

void printLSTableForNode(int i)
{
    printf("\nLINK STATE ROUTING TABLE for Router %s\n", name[i]);
    printf("%-15s %-10s %-10s %-20s\n", "Destination", "Cost", "NextHop", "Shortest Path");
    printf("----------------------------------------------------------------------\n");
    for (int j = 0; j < n; j++)
    {
        if (i == j)
            continue;
        printf("%-15s", name[j]);
        if (lsDist[i][j] >= INF)
        {
            printf("%-10s %-10s %-20s\n", "INF", "-", "-");
        }
        else
        {
            printf("%-10d %-10s ", lsDist[i][j], name[lsNextHop[i][j]]);
            printPath(i, j, lsNextHop);
            printf("\n");
        }
    }
}

void displayLinkStateMenu()
{
    int option;
    printf("\nLink State Views:\n1. View single node\n2. View all nodes at once\nEnter choice: ");
    scanf("%d", &option);

    if (option == 1)
    {
        char searchName[20];
        printf("Enter node name: ");
        scanf("%s", searchName);
        int i = findNodeIndex(searchName);
        if (i == -1) printf("Node not found!\n");
        else printLSTableForNode(i);
    }
    else if (option == 2)
    {
        for (int i = 0; i < n; i++)
            printLSTableForNode(i);
    }
    else
    {
        printf("Invalid View Option.\n");
    }
}

int main()
{
    int choice;
    readInput();
    do
    {
        printf("\n\n");
        printf("---------------------------------------------\n");
        printf("          ROUTING ALGORITHMS\n");
        printf("---------------------------------------------\n");
        printf("1. View Initial Routing Table (Any Node)\n");
        printf("2. Distance Vector Routing (Run & Choose View)\n");
        printf("3. Link State Routing (Run & Choose View)\n");
        printf("4. Exit\n");
        printf("---------------------------------------------\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        switch (choice)
        {
            case 1:
                displayInitialTable();
                break;
            case 2:
                initDistanceVector();
                runDistanceVector();
                displayDistanceVectorMenu();
                break;
            case 3:
                runLinkState();
                displayLinkStateMenu();
                break;
            case 4:
                printf("\nProgram terminated.\n");
                break;
            default:
                printf("\nInvalid choice! Please try again.\n");
        }
    } while (choice != 4);
    return 0;
}
