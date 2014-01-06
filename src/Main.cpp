/****************************************************************************************************
    Solution to the problem "Mortal Coil" at http://www.hacker.org/coil
    Author: Eric Gopak

# Ideas:
    o Stop if there is isolated exit (check if mask contains isolated bit, like xxx010xxx)
    o Use Pits (level->Ends) as one-sized temporaryEndBlock
    o Force Simulator::backtrack() visit every cell only ONCE!
    o Pruning: if new exit got blocked - check if there are at least 2 exits left (if it's not the very last component)
    o Redefine 'exits' to be the ones that cannot be blocked during solve()
    o Disallow ANY 'tails' in analysis solutions

# Optimization ideas:
    o Rewrite std::set with std::unordered_set. Better with std::vector. Best with C-style array.
    o Rewrite STL containers with custom ones (maybe just replace std::map with std::unordered_map)
    o Make use of SSE/SSE2 instructions
    o Play around with compiler flags
    o Try compiling with GCC, Intel C++, Clang compilers
    o see http://msdn.microsoft.com/en-us/library/y0dh78ez%28VS.80%29.aspx

# Figure out how to:
    o filter solutions if pits appear (exits are split into separated sets)

# Notes
    o No SPECIAL components in following levels: 24, 28
    o Interesting largest component in Level 28 (2 portals to itself)

****************************************************************************************************/

#include "Common.h"
#include "Colorer.h"
#include "Obstacle.h"
#include "Component.h"
#include "Analyzer.h"
#include "Solver.h"

static const char* INPUT_FILENAME = "mortal_coil.txt";
static const char* OUTPUT_FILENAME = "output.txt";

void printUsage()
{
    Colorer::print<YELLOW>(
        "Usage:\n"
        "MortalCoil.exe\n"
        "MortalCoil.exe row col [--special component-id]\n"
        "\n"
    );
}

bool readArguments(int argc, char* argv[], int* row, int* col, int* firstComponentId)
{
    if (argc == 3)
    {
        if (sscanf(argv[1], "%d", row) != 1)
        {
            return false;
        }

        if (sscanf(argv[2], "%d", col) != 1)
        {
            return false;
        }
    }
    else if (argc >= 4)
    {
        // starting from component with given index
        if (sscanf(argv[3], "%d", firstComponentId) != 1)
        {
            return false;
        }
    }

    return true;
}

int main(int argc, char* argv[])
{
    /*Colorer::print<  0>("This is color %2x\n",  0x0);
    Colorer::print<  1>("This is color %2x\n",  0x1);
    Colorer::print<  2>("This is color %2x\n",  0x2);
    Colorer::print<  3>("This is color %2x\n",  0x3);
    Colorer::print<  4>("This is color %2x\n",  0x4);
    Colorer::print<  5>("This is color %2x\n",  0x5);
    Colorer::print<  6>("This is color %2x\n",  0x6);
    Colorer::print<  7>("This is color %2x\n",  0x7);
    Colorer::print<  8>("This is color %2x\n",  0x8);
    Colorer::print<  9>("This is color %2x\n",  0x9);
    Colorer::print< 10>("This is color %2x\n",  0xa);
    Colorer::print< 11>("This is color %2x\n",  0xb);
    Colorer::print< 12>("This is color %2x\n",  0xc);
    Colorer::print< 13>("This is color %2x\n",  0xd);
    Colorer::print< 14>("This is color %2x\n",  0xe);
    Colorer::print< 15>("This is color %2x\n",  0xf);
    Colorer::print< 16>("This is color %2x\n", 0x10);
    Colorer::print< 17>("This is color %2x\n", 0x11);
    Colorer::print< 18>("This is color %2x\n", 0x12);
    Colorer::print< 19>("This is color %2x\n", 0x13);
    Colorer::print< 20>("This is color %2x\n", 0x14);
    Colorer::print< 21>("This is color %2x\n", 0x15);
    Colorer::print< 22>("This is color %2x\n", 0x16);
    Colorer::print< 23>("This is color %2x\n", 0x17);
    Colorer::print< 24>("This is color %2x\n", 0x18);
    Colorer::print< 25>("This is color %2x\n", 0x19);
    Colorer::print< 26>("This is color %2x\n", 0x1a);
    Colorer::print< 27>("This is color %2x\n", 0x1b);
    Colorer::print< 28>("This is color %2x\n", 0x1c);
    Colorer::print< 29>("This is color %2x\n", 0x1d);
    Colorer::print< 30>("This is color %2x\n", 0x1e);
    Colorer::print< 31>("This is color %2x\n", 0x1f);
    Colorer::print< 32>("This is color %2x\n", 0x20);
    Colorer::print< 33>("This is color %2x\n", 0x21);
    Colorer::print< 34>("This is color %2x\n", 0x22);
    Colorer::print< 35>("This is color %2x\n", 0x23);
    Colorer::print< 36>("This is color %2x\n", 0x24);
    Colorer::print< 37>("This is color %2x\n", 0x25);
    Colorer::print< 38>("This is color %2x\n", 0x26);
    Colorer::print< 39>("This is color %2x\n", 0x27);
    Colorer::print< 40>("This is color %2x\n", 0x28);
    Colorer::print< 41>("This is color %2x\n", 0x29);
    Colorer::print< 42>("This is color %2x\n", 0x2a);
    Colorer::print< 43>("This is color %2x\n", 0x2b);
    Colorer::print< 44>("This is color %2x\n", 0x2c);
    Colorer::print< 45>("This is color %2x\n", 0x2d);
    Colorer::print< 46>("This is color %2x\n", 0x2e);
    Colorer::print< 47>("This is color %2x\n", 0x2f);
    Colorer::print< 48>("This is color %2x\n", 0x30);
    Colorer::print< 49>("This is color %2x\n", 0x31);
    Colorer::print< 50>("This is color %2x\n", 0x32);
    Colorer::print< 51>("This is color %2x\n", 0x33);
    Colorer::print< 52>("This is color %2x\n", 0x34);
    Colorer::print< 53>("This is color %2x\n", 0x35);
    Colorer::print< 54>("This is color %2x\n", 0x36);
    Colorer::print< 55>("This is color %2x\n", 0x37);
    Colorer::print< 56>("This is color %2x\n", 0x38);
    Colorer::print< 57>("This is color %2x\n", 0x39);
    Colorer::print< 58>("This is color %2x\n", 0x3a);
    Colorer::print< 59>("This is color %2x\n", 0x3b);
    Colorer::print< 60>("This is color %2x\n", 0x3c);
    Colorer::print< 61>("This is color %2x\n", 0x3d);
    Colorer::print< 62>("This is color %2x\n", 0x3e);
    Colorer::print< 63>("This is color %2x\n", 0x3f);
    Colorer::print< 64>("This is color %2x\n", 0x40);
    Colorer::print< 65>("This is color %2x\n", 0x41);
    Colorer::print< 66>("This is color %2x\n", 0x42);
    Colorer::print< 67>("This is color %2x\n", 0x43);
    Colorer::print< 68>("This is color %2x\n", 0x44);
    Colorer::print< 69>("This is color %2x\n", 0x45);
    Colorer::print< 70>("This is color %2x\n", 0x46);
    Colorer::print< 71>("This is color %2x\n", 0x47);
    Colorer::print< 72>("This is color %2x\n", 0x48);
    Colorer::print< 73>("This is color %2x\n", 0x49);
    Colorer::print< 74>("This is color %2x\n", 0x4a);
    Colorer::print< 75>("This is color %2x\n", 0x4b);
    Colorer::print< 76>("This is color %2x\n", 0x4c);
    Colorer::print< 77>("This is color %2x\n", 0x4d);
    Colorer::print< 78>("This is color %2x\n", 0x4e);
    Colorer::print< 79>("This is color %2x\n", 0x4f);
    Colorer::print< 80>("This is color %2x\n", 0x50);
    Colorer::print< 81>("This is color %2x\n", 0x51);
    Colorer::print< 82>("This is color %2x\n", 0x52);
    Colorer::print< 83>("This is color %2x\n", 0x53);
    Colorer::print< 84>("This is color %2x\n", 0x54);
    Colorer::print< 85>("This is color %2x\n", 0x55);
    Colorer::print< 86>("This is color %2x\n", 0x56);
    Colorer::print< 87>("This is color %2x\n", 0x57);
    Colorer::print< 88>("This is color %2x\n", 0x58);
    Colorer::print< 89>("This is color %2x\n", 0x59);
    Colorer::print< 90>("This is color %2x\n", 0x5a);
    Colorer::print< 91>("This is color %2x\n", 0x5b);
    Colorer::print< 92>("This is color %2x\n", 0x5c);
    Colorer::print< 93>("This is color %2x\n", 0x5d);
    Colorer::print< 94>("This is color %2x\n", 0x5e);
    Colorer::print< 95>("This is color %2x\n", 0x5f);
    Colorer::print< 96>("This is color %2x\n", 0x60);
    Colorer::print< 97>("This is color %2x\n", 0x61);
    Colorer::print< 98>("This is color %2x\n", 0x62);
    Colorer::print< 99>("This is color %2x\n", 0x63);
    Colorer::print<100>("This is color %2x\n", 0x64);
    Colorer::print<101>("This is color %2x\n", 0x65);
    Colorer::print<102>("This is color %2x\n", 0x66);
    Colorer::print<103>("This is color %2x\n", 0x67);
    Colorer::print<104>("This is color %2x\n", 0x68);
    Colorer::print<105>("This is color %2x\n", 0x69);
    Colorer::print<106>("This is color %2x\n", 0x6a);
    Colorer::print<107>("This is color %2x\n", 0x6b);
    Colorer::print<108>("This is color %2x\n", 0x6c);
    Colorer::print<109>("This is color %2x\n", 0x6d);
    Colorer::print<110>("This is color %2x\n", 0x6e);
    Colorer::print<111>("This is color %2x\n", 0x6f);
    Colorer::print<112>("This is color %2x\n", 0x70);
    Colorer::print<113>("This is color %2x\n", 0x71);
    Colorer::print<114>("This is color %2x\n", 0x72);
    Colorer::print<115>("This is color %2x\n", 0x73);
    Colorer::print<116>("This is color %2x\n", 0x74);
    Colorer::print<117>("This is color %2x\n", 0x75);
    Colorer::print<118>("This is color %2x\n", 0x76);
    Colorer::print<119>("This is color %2x\n", 0x77);
    Colorer::print<120>("This is color %2x\n", 0x78);
    Colorer::print<121>("This is color %2x\n", 0x79);
    Colorer::print<122>("This is color %2x\n", 0x7a);
    Colorer::print<123>("This is color %2x\n", 0x7b);
    Colorer::print<124>("This is color %2x\n", 0x7c);
    Colorer::print<125>("This is color %2x\n", 0x7d);
    Colorer::print<126>("This is color %2x\n", 0x7e);
    Colorer::print<127>("This is color %2x\n", 0x7f);
    Colorer::print<128>("This is color %2x\n", 0x80);
    Colorer::print<129>("This is color %2x\n", 0x81);
    Colorer::print<130>("This is color %2x\n", 0x82);
    Colorer::print<131>("This is color %2x\n", 0x83);
    Colorer::print<132>("This is color %2x\n", 0x84);
    Colorer::print<133>("This is color %2x\n", 0x85);
    Colorer::print<134>("This is color %2x\n", 0x86);
    Colorer::print<135>("This is color %2x\n", 0x87);
    Colorer::print<136>("This is color %2x\n", 0x88);
    Colorer::print<137>("This is color %2x\n", 0x89);
    Colorer::print<138>("This is color %2x\n", 0x8a);
    Colorer::print<139>("This is color %2x\n", 0x8b);
    Colorer::print<140>("This is color %2x\n", 0x8c);
    Colorer::print<141>("This is color %2x\n", 0x8d);
    Colorer::print<142>("This is color %2x\n", 0x8e);
    Colorer::print<143>("This is color %2x\n", 0x8f);
    Colorer::print<144>("This is color %2x\n", 0x90);
    Colorer::print<145>("This is color %2x\n", 0x91);
    Colorer::print<146>("This is color %2x\n", 0x92);
    Colorer::print<147>("This is color %2x\n", 0x93);
    Colorer::print<148>("This is color %2x\n", 0x94);
    Colorer::print<149>("This is color %2x\n", 0x95);
    Colorer::print<150>("This is color %2x\n", 0x96);
    Colorer::print<151>("This is color %2x\n", 0x97);
    Colorer::print<152>("This is color %2x\n", 0x98);
    Colorer::print<153>("This is color %2x\n", 0x99);
    Colorer::print<154>("This is color %2x\n", 0x9a);
    Colorer::print<155>("This is color %2x\n", 0x9b);
    Colorer::print<156>("This is color %2x\n", 0x9c);
    Colorer::print<157>("This is color %2x\n", 0x9d);
    Colorer::print<158>("This is color %2x\n", 0x9e);
    Colorer::print<159>("This is color %2x\n", 0x9f);
    Colorer::print<160>("This is color %2x\n", 0xa0);
    Colorer::print<161>("This is color %2x\n", 0xa1);
    Colorer::print<162>("This is color %2x\n", 0xa2);
    Colorer::print<163>("This is color %2x\n", 0xa3);
    Colorer::print<164>("This is color %2x\n", 0xa4);
    Colorer::print<165>("This is color %2x\n", 0xa5);
    Colorer::print<166>("This is color %2x\n", 0xa6);
    Colorer::print<167>("This is color %2x\n", 0xa7);
    Colorer::print<168>("This is color %2x\n", 0xa8);
    Colorer::print<169>("This is color %2x\n", 0xa9);
    Colorer::print<170>("This is color %2x\n", 0xaa);
    Colorer::print<171>("This is color %2x\n", 0xab);
    Colorer::print<172>("This is color %2x\n", 0xac);
    Colorer::print<173>("This is color %2x\n", 0xad);
    Colorer::print<174>("This is color %2x\n", 0xae);
    Colorer::print<175>("This is color %2x\n", 0xaf);
    Colorer::print<176>("This is color %2x\n", 0xb0);
    Colorer::print<177>("This is color %2x\n", 0xb1);
    Colorer::print<178>("This is color %2x\n", 0xb2);
    Colorer::print<179>("This is color %2x\n", 0xb3);
    Colorer::print<180>("This is color %2x\n", 0xb4);
    Colorer::print<181>("This is color %2x\n", 0xb5);
    Colorer::print<182>("This is color %2x\n", 0xb6);
    Colorer::print<183>("This is color %2x\n", 0xb7);
    Colorer::print<184>("This is color %2x\n", 0xb8);
    Colorer::print<185>("This is color %2x\n", 0xb9);
    Colorer::print<186>("This is color %2x\n", 0xba);
    Colorer::print<187>("This is color %2x\n", 0xbb);
    Colorer::print<188>("This is color %2x\n", 0xbc);
    Colorer::print<189>("This is color %2x\n", 0xbd);
    Colorer::print<190>("This is color %2x\n", 0xbe);
    Colorer::print<191>("This is color %2x\n", 0xbf);
    Colorer::print<192>("This is color %2x\n", 0xc0);
    Colorer::print<193>("This is color %2x\n", 0xc1);
    Colorer::print<194>("This is color %2x\n", 0xc2);
    Colorer::print<195>("This is color %2x\n", 0xc3);
    Colorer::print<196>("This is color %2x\n", 0xc4);
    Colorer::print<197>("This is color %2x\n", 0xc5);
    Colorer::print<198>("This is color %2x\n", 0xc6);
    Colorer::print<199>("This is color %2x\n", 0xc7);
    Colorer::print<200>("This is color %2x\n", 0xc8);
    Colorer::print<201>("This is color %2x\n", 0xc9);
    Colorer::print<202>("This is color %2x\n", 0xca);
    Colorer::print<203>("This is color %2x\n", 0xcb);
    Colorer::print<204>("This is color %2x\n", 0xcc);
    Colorer::print<205>("This is color %2x\n", 0xcd);
    Colorer::print<206>("This is color %2x\n", 0xce);
    Colorer::print<207>("This is color %2x\n", 0xcf);
    Colorer::print<208>("This is color %2x\n", 0xd0);
    Colorer::print<209>("This is color %2x\n", 0xd1);
    Colorer::print<210>("This is color %2x\n", 0xd2);
    Colorer::print<211>("This is color %2x\n", 0xd3);
    Colorer::print<212>("This is color %2x\n", 0xd4);
    Colorer::print<213>("This is color %2x\n", 0xd5);
    Colorer::print<214>("This is color %2x\n", 0xd6);
    Colorer::print<215>("This is color %2x\n", 0xd7);
    Colorer::print<216>("This is color %2x\n", 0xd8);
    Colorer::print<217>("This is color %2x\n", 0xd9);
    Colorer::print<218>("This is color %2x\n", 0xda);
    Colorer::print<219>("This is color %2x\n", 0xdb);
    Colorer::print<220>("This is color %2x\n", 0xdc);
    Colorer::print<221>("This is color %2x\n", 0xdd);
    Colorer::print<222>("This is color %2x\n", 0xde);
    Colorer::print<223>("This is color %2x\n", 0xdf);
    Colorer::print<224>("This is color %2x\n", 0xe0);
    Colorer::print<225>("This is color %2x\n", 0xe1);
    Colorer::print<226>("This is color %2x\n", 0xe2);
    Colorer::print<227>("This is color %2x\n", 0xe3);
    Colorer::print<228>("This is color %2x\n", 0xe4);
    Colorer::print<229>("This is color %2x\n", 0xe5);
    Colorer::print<230>("This is color %2x\n", 0xe6);
    Colorer::print<231>("This is color %2x\n", 0xe7);
    Colorer::print<232>("This is color %2x\n", 0xe8);
    Colorer::print<233>("This is color %2x\n", 0xe9);
    Colorer::print<234>("This is color %2x\n", 0xea);
    Colorer::print<235>("This is color %2x\n", 0xeb);
    Colorer::print<236>("This is color %2x\n", 0xec);
    Colorer::print<237>("This is color %2x\n", 0xed);
    Colorer::print<238>("This is color %2x\n", 0xee);
    Colorer::print<239>("This is color %2x\n", 0xef);
    Colorer::print<240>("This is color %2x\n", 0xf0);
    Colorer::print<241>("This is color %2x\n", 0xf1);
    Colorer::print<242>("This is color %2x\n", 0xf2);
    Colorer::print<243>("This is color %2x\n", 0xf3);
    Colorer::print<244>("This is color %2x\n", 0xf4);
    Colorer::print<245>("This is color %2x\n", 0xf5);
    Colorer::print<246>("This is color %2x\n", 0xf6);
    Colorer::print<247>("This is color %2x\n", 0xf7);
    Colorer::print<248>("This is color %2x\n", 0xf8);
    Colorer::print<249>("This is color %2x\n", 0xf9);
    Colorer::print<250>("This is color %2x\n", 0xfa);
    Colorer::print<251>("This is color %2x\n", 0xfb);
    Colorer::print<252>("This is color %2x\n", 0xfc);
    Colorer::print<253>("This is color %2x\n", 0xfd);
    Colorer::print<254>("This is color %2x\n", 0xfe);
    Colorer::print<255>("This is color %2x\n", 0xff);
    return 0;*/

    clock_t start_time = clock();

    int row = -1, col = -1;
    int firstComponentId = -1;

    if (!readArguments(argc, argv, &row, &col, &firstComponentId))
    {
        printUsage();
        return 0;
    }

    Level level(INPUT_FILENAME);

    TRACE(Colorer::print<RED>("Preprocessing...\n"));

    Analyzer analyzer(&level);
    Solver solver(&level, "output.txt");
    analyzer.preprocess();

    level.traceComponent();

    TRACE
    (
        printf("Free cells: %d\n", level.Free);
        printf("Components: %d\n", level.getObstacleCount());

        //level.traceComponent();

        if (level.initialEnds.size() > 0)
        {
            printf("Initial tails: %d\n", level.initialEnds.size());
            for (size_t e = 0; e < level.initialEnds.size(); e++)
            {
                printf("Tail%d: %d %d\n", e + 1, level.initialEnds[e]->getX(), level.initialEnds[e]->getY());
            }
        }
        if (level.getTemporaryEndBlocks().size() > 0)
        {
            printf("Initial end blocks: %d\n", level.getTemporaryEndBlocks().size());
        }
    );

    TRACE(Colorer::print<RED>("Analyzing...\n"));

    analyzer.analyzeComponents();

#ifdef TRACE_STATISTICS
    Colorer::print<WHITE>("Max number of solutions: %d\n", Debug::mostSolutions);
#endif

    TRACE(Colorer::print<RED>("Solving...\n"));

    int firstRow = (row != -1) ? row : 1;
    int firstCol = (col != -1) ? col : 1;
    solver.solve(firstRow, firstCol, firstComponentId);

    TRACE(Colorer::print<WHITE>(level.Solved ? "SOLVED\n" : "FAILED TO SOLVE\n"));

#ifdef TRACE_STATISTICS
    printf("Got too many temporary ends %d times!\n", Debug::gotTooManyTemporaryEndsCounter);
    printf("Got invalid next touches %d times!\n", Debug::gotInvalidNextTouchesCounter);
    printf("Got isolated cells %d times!\n", Debug::gotIsolatedCellsCounter);
    printf("Got too many temporary end blocks %d times!\n", Debug::gotTooManyTemporaryEndBlocksCounter);
#endif

    if (level.Solved)
    {
        level.outputToFile(OUTPUT_FILENAME);
    }

    clock_t finish_time = clock();
    printf("%lf\n", double(finish_time - start_time) / CLOCKS_PER_SEC);

    return 0;
}
