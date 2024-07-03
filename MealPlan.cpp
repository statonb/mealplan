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

const char *menuFileName = "/mnt/omv1share1/Workspace/MealPlan/menu.txt";
const char *recentSelectionsFileName = "/mnt/omv1share1/Workspace/MealPlan/recentSelections.txt";
const size_t DEFAULT_MAX_NUM_RECENT_SELECTIONS = 14;

bool isInVector(std::vector<int> *pV, int target)
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
int main1(void)
{
    int selection;
    FILE *fpMenu;
    FILE *fpRecentSelections;
    char *cp;
    char tempLine[256];
    std::vector<std::string> theMenu;
    std::vector<int> recentSelections;
    std::vector<int>::const_iterator itRecentSelections;
    bool exitFlag = false;
    bool acceptFlag = false;
    bool abortFlag = false;
    char acceptYNQ;
    size_t maxNumRecentSelections = DEFAULT_MAX_NUM_RECENT_SELECTIONS;
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
        theMenu.push_back(tempLine);
    }
    fclose(fpMenu);

    fpRecentSelections = fopen(recentSelectionsFileName, "r");
    if ((FILE *)(NULL) != fpRecentSelections)
    {
        while(fgets(tempLine, sizeof(tempLine), fpRecentSelections))
        {
            selection = (int)strtol(tempLine, NULL, 10);
            recentSelections.push_back(selection);
        }
        fclose(fpRecentSelections);
    }

    tcgetattr(STDIN_FILENO, &oldSettings);
    newSettings = oldSettings;
    newSettings.c_lflag &= (~ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);

    srand(time(NULL));

    while   (   (false == acceptFlag)
             && (false == abortFlag)
            )
    {
        exitFlag = false;
        while (false == exitFlag)
        {
            selection = rand() % theMenu.size();
            if (false == isInVector(&recentSelections, selection))
            {
                std::cout << '\n' << selection << ": " << theMenu[selection] << '\n';
                exitFlag = true;
            }
        }
        printf("Accept? <Y/N/Q>: ");
        acceptYNQ = (char)getchar();

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
        putchar('\n');
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
    itRecentSelections = recentSelections.begin();
    if  (   (itRecentSelections != recentSelections.end())
         && (recentSelections.size() >= maxNumRecentSelections)
        )
    {
        itRecentSelections++;
    }
    while (itRecentSelections != recentSelections.end())
    {
        fprintf(fpRecentSelections, "%d\n", *itRecentSelections);
        itRecentSelections++;
    }
    fprintf(fpRecentSelections, "%d\n", selection);
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
