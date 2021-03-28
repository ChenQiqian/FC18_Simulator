#include "game/game.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
using namespace std;

Game *g;

int main(int argc, char **argv) {
    int id = time(0), w = 15, h = 15, p = 4;
    if(argc != 1) {
        sscanf(argv[1], "%d", &id);
    }
    g = new Game(w, h, p, id);
    string f[p], js[2];
    // clang-format off
    cerr << "Generated Game map. TIMEID=" << id << "." << endl;
    cerr << "player1 file(./player/player1.exe):" ;
    getline(cin,f[0]);if(f[0].length() == 0) f[0] = "./player/player1.exe";
    cerr << "player2 file(./player/player2.exe):" ;
    getline(cin,f[1]);if(f[1].length() == 0) f[1] = "./player/player2.exe";
    cerr << "player3 file(./player/player3.exe):" ;
    getline(cin,f[2]);if(f[2].length() == 0) f[2] = "./player/player3.exe";
    cerr << "player4 file(./player/player4.exe):" ;
    getline(cin,f[3]);if(f[3].length() == 0) f[3] = "./player/player4.exe";
    
    cerr << "info json file path(./info.json):" ;
    getline(cin,js[0]);if(js[0].length() == 0) js[0] = "./info.json";
    cerr << "command json file path(./command.json):" ;
    getline(cin,js[1]);if(js[1].length() == 0) js[1] = "./command.json";
    // clang-format on
    // cerr << f[0] << f[1] << f[2] << f[3];

    system(("mkdir ./logs/game" + to_string(id)).c_str());
    cerr << "Created log dir." << endl;
    cerr << "-------------------" << endl;
    cerr << "Start Game." << endl;

    fstream          infoFile, commandFile;
    string           commandString;
    Json::FastWriter writer;
    Json::Reader     reader;
    for(int i = 0; i < 100; i++) {
        cerr << "Round " << i << ":" << endl;
        string _t = "echo \"round " + to_string(i) + ":\" >> logs/game" +
            to_string(id) + "/command.json";
        system(_t.c_str());

        for(int _p = 1; _p <= p; _p++) {
            g->totalRounds++;
            g->myID = _p;
            // output

            string infoString = writer.write(g->asJson());
            infoFile.open(js[0], ios::out);
            infoFile << infoString;
            infoFile.close();

            // read
            string _command = f[_p - 1] + " < " + js[0] + " > " + js[1];
            // cerr << "command: " << _command << endl;
            system(_command.c_str());

            // do command
            CommandList cList;
            commandFile.open(js[1], ios::in);
            commandFile >> commandString;
            commandFile.close();
            Json::Value commandJson;
            reader.parse(commandString.data(), commandJson);
            cList = commandJson;
            g->play(_p, cList);

            // save info and command

            string tmp;
            tmp =
                "cat " + js[0] + " >> logs/game" + to_string(id) + "/info.json";
            // cerr << "save info: " << tmp << endl;
            // system(tmp.c_str());
            tmp = "echo \"player " + to_string(_p) + ":\" >> logs/game" +
                to_string(id) + "/command.json";
            system(tmp.c_str());
            tmp = "cat " + js[1] + " >> logs/game" + to_string(id) +
                "/command.json";
            // cerr << "save command: " << tmp << endl;
            system(tmp.c_str());
        }
        g->print();
        cerr << "\tScore: ";
        for(int _p = 1; _p <= p; _p++) {
            cerr << "player " << _p << " " << g->playerInfo[_p - 1].score
                 << "; ";
        }
        cerr << endl;
        // break;
    }
    // clean files.
    cerr << "Remove tmp files." << endl;
    // system(("rm " + js[0]).c_str());
    // system(("rm " + js[1]).c_str());
    return 0;
}