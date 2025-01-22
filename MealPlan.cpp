#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <termios.h>
#include <vector>
#include <string>
#include <map>
#include <set>
#include "crc32.h"

const char *menuFileName = "/mnt/omv1share1/Workspace/MealPlan/menu.txt";
const char *recentSelectionsFileName = "/mnt/omv1share1/Workspace/MealPlan/recentSelections.txt";
const size_t DEFAULT_MAX_NUM_RECENT_SELECTIONS = 10;

bool isInVector(std::vector<int> *pV, const int target)
{
    std::vector<int>::const_iterator itV = pV->begin();
    while (itV != pV->end())
    {
        if (target == *itV)
        {
            return true;
        }
        itV++;
    }
    return false;
}

void randInit(bool verboseFlag = false)
{
    uint32_t t;
    uint32_t pid;
    uint32_t randSeed = 0;

    t = (uint32_t)time(NULL);
    pid = (uint32_t)getpid();
    randSeed = crc32(randSeed, &t, sizeof(t));
    randSeed = crc32(randSeed, &pid, sizeof(pid));
    if (verboseFlag)
    {
        printf("t = %08X  pid = %08X  srand(%08X)\n", t, pid, randSeed);
    }
    srand(randSeed);
}

int main1(void)
{
    int selection;
    FILE *fpMenu;
    FILE *fpRecentSelections;
    char *cp;
    char tempLine[256];
    std::vector<std::string> theMenu;
    std::vector<int> recentSelectionsVector;
    std::vector<int> rejectedItemsVector;
    std::vector<int>::const_iterator itRecentSelectionsVector;
    bool exitFlag = false;
    bool acceptFlag = false;
    bool abortFlag = false;
    bool nullMenuItem;
    char acceptYNQ;
    size_t maxNumRecentSelections = DEFAULT_MAX_NUM_RECENT_SELECTIONS;
    size_t itemsRemaining;
    struct termios oldSettings;
    struct termios newSettings;

    fpMenu = fopen(menuFileName, "r");
    if ((FILE *)(NULL) == fpMenu)
    {
        fprintf(stderr, "Can't open menu file.\n");
        return 1;
    }
    while (fgets(tempLine, sizeof(tempLine), fpMenu))
    {
        cp = strrchr(tempLine, '\n');
        if ((char *)(NULL) != cp)
        {
            *cp = '\0';
        }
        // If there's a colon on the line then the line must start with an index number as follows:
        // 5: The Fifth Menu Item
        // Skip over this index number and the space that follows the colon.
        cp = strchr(tempLine, ':');
        if ((char *)(NULL) != cp)
        {
            // Found the colon.  Advance past the colon and the space
            cp += 2;
        }
        else
        {
            // No colon found.
            cp = tempLine;
        }
        theMenu.push_back(cp);
    }
    fclose(fpMenu);

    fpRecentSelections = fopen(recentSelectionsFileName, "r");
    if ((FILE *)(NULL) != fpRecentSelections)
    {
        while(fgets(tempLine, sizeof(tempLine), fpRecentSelections))
        {
            // First thing on the line is a selection number.  The rest of the line can be anything
            // as long as there is a colon or a space immediately after the selection number.
            cp = strchr(tempLine, ':');
            if ((char *)(NULL) != cp)
            {
                *cp = '\0';
            }
            else
            {
                cp = strchr(tempLine, ' ');
                if ((char *)(NULL) != cp)
                {
                    *cp = '\0';
                }
            }
            selection = (int)strtol(tempLine, NULL, 10);
            recentSelectionsVector.push_back(selection);
        }
        fclose(fpRecentSelections);
    }

    itemsRemaining = theMenu.size() - recentSelectionsVector.size();

    tcgetattr(STDIN_FILENO, &oldSettings);
    newSettings = oldSettings;
    newSettings.c_lflag &= (~ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);

    randInit();

    while   (   (false == acceptFlag)
             && (false == abortFlag)
            )
    {
        exitFlag = false;
        while (false == exitFlag)
        {
            selection = rand() % theMenu.size();
            if  (   (false == isInVector(&recentSelectionsVector, selection))
                 && (false == isInVector(&rejectedItemsVector, selection))
                )
            {
                // See if the selection contains the string "null".
                nullMenuItem = ((char *)(NULL) != strstr(theMenu[selection].c_str(), "null")) ? true : false;
                if (false == nullMenuItem)
                {
                    std::cout << '\n' << selection << ": " << theMenu[selection] << '\n';
                }
                exitFlag = true;
            }
        }

        if (true == nullMenuItem)
        {
            // The selection contains the string "null" so skip it
            acceptYNQ = 'n';
        }
        else
        {
            printf("Accept? <Y/N/Q>: ");
            acceptYNQ = (char)getchar();
            putchar('\n');
        }

        if  (   ('Q' == acceptYNQ)
             || ('q' == acceptYNQ)
            )
        {
            abortFlag = true;
        }
        else if (   ('Y' == acceptYNQ)
                 || ('y' == acceptYNQ)
                )
        {
            acceptFlag = true;
        }
        else
        {
            --itemsRemaining;
            if (itemsRemaining > 0)
            {
                rejectedItemsVector.push_back(selection);
            }
            else
            {
                abortFlag = true;
            }
            if  (   (false == nullMenuItem)
                 || (0 == itemsRemaining)
                )
            {
                printf("\n%ld menu items remaining\n", itemsRemaining);
            }
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);

    if (abortFlag)
    {
        return 0;
    }

    fpRecentSelections = fopen(recentSelectionsFileName, "w");
    if ((FILE *)(NULL) == fpRecentSelections)
    {
        fprintf(stderr, "Can't open recent selections file.\n");
        return 2;
    }
    // Write current selection first
    fprintf(fpRecentSelections, "%d: %s\n", selection, theMenu[selection].c_str());

    // Write recent selections up to the max number of saved selections
    itRecentSelectionsVector = recentSelectionsVector.begin();
    size_t n = 1;
    while   (   (itRecentSelectionsVector != recentSelectionsVector.end())
             && (n < maxNumRecentSelections)
            )
    {
        fprintf(fpRecentSelections, "%d: %s\n", *itRecentSelectionsVector, theMenu[*itRecentSelectionsVector].c_str());
        itRecentSelectionsVector++;
        n++;
    }
    fclose(fpRecentSelections);

    return 0;
}

int main2(void)
{
    return 0;
}

int main(int argc, char *argv[])
{
    return (main1());
}
