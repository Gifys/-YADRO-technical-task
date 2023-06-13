#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <queue>


int countPC, occupiedPC = 0, priceH, timeStart, timeEnd;

std::map<int, std::pair<std::string, std::string>> tablesForward; // стол - (время, имя)
std::map<std::string, int> tablesBack; // имя - стол
std::set<std::string> camePeople;
std::queue<std::string> quOnTables;
std::map<int, std::pair<int, int>> score; // комп - (счет, общее время)

int TimeToInt(std::string time){
    int ans = 0;
    ans += ((time[0] - '0') * 10 + (time[1] - '0'))*60 + (time[3] - '0') * 10 + (time[4] - '0');
    return ans;
}

std::string TimeToStr(int time){
    std::string h, m;
    h = std::to_string(time/60); m = std::to_string(time%60);

    while (h.length() < 2)
        h = "0"+h;
    while (m.length() < 2)
        m = "0"+m;

    return h+':'+m;
}

class OutgoingEvents{
    protected:
        void id11(std::string time, std::string name){
            // std::cout << time << " 11 " << name << std::endl;

            camePeople.erase(camePeople.find(name));

            if (tablesBack[name] != -1){
                int diffTime = TimeToInt(time)-TimeToInt(tablesForward[tablesBack[name]].first);
                int cash = (diffTime+59)/60*priceH;

                score[tablesBack[name]].first += cash;
                score[tablesBack[name]].second += diffTime;


                tablesForward[tablesBack[name]].first = "";
                tablesForward[tablesBack[name]].second = "";
                occupiedPC--;
            }
        }
        void id12(std::string time, std::string name){
            if (tablesBack[name] != -1){
                if (!quOnTables.empty()){
                    std::string newPer = quOnTables.front();
                    quOnTables.pop();

                    id11(time, name);
                    occupiedPC++; // А то в id 11 вы его вычитаем
                    
                    tablesForward[tablesBack[name]].first = time;
                    tablesForward[tablesBack[name]].second = newPer;
                    tablesBack[newPer] = tablesBack[name];
                    tablesBack.erase(tablesBack.find(name));

                    std::cout << time << " 12 " << newPer << ' ' << tablesBack[newPer]+1 << std::endl;
                }
                else{
                    id11(time, name);
                    tablesForward[tablesBack[name]].second = "";
                    tablesBack.erase(tablesBack.find(name));
                }
            }

        }
        std::string id13(std::string time, std::string error){
            return time + " 13 " + error;
        }

};

class IncomingEvents: protected OutgoingEvents{
    protected:
        void id1(std::string time, std::string name){
            std::cout << time << " 1 " << name << std::endl;

            int coming = TimeToInt(time);
            if (coming < timeStart || coming > timeEnd)
                std::cout << id13(time, "NotOpenYet") << std::endl;
            else if (camePeople.find(name) != camePeople.end())
                std::cout << id13(time, "YouShallNotPass") << std::endl;
            else{
                camePeople.insert(name);
                tablesBack[name] = -1;
            }
                
        }
        void id2(std::string time, std::string name, int numT){
            std::cout << time << " 2 " << name << ' ' << numT << std::endl;
            numT--;

            if (camePeople.find(name) == camePeople.end())
                std::cout << id13(time, "ClientUnknown") << std::endl;
            else if (tablesForward[numT].second != "")
                std::cout << id13(time, "PlaceIsBusy") << std::endl;
            else{
                if (tablesBack[name] != -1){
                    tablesForward[tablesBack[name]].second = "";
                    occupiedPC--;
                }
                tablesForward[numT].first = time;
                tablesForward[numT].second = name;
                tablesBack[name] = numT;
                occupiedPC++;
            }
        }
        void id3(std::string time, std::string name){
            std::cout << time << " 3 " << name << std::endl;

            if (countPC > occupiedPC)
                std::cout << id13(time, "ICanWaitNoLonger!") << std::endl;
            else if (countPC <= quOnTables.size()){
                std::cout << time << " 11 " << name << std::endl;
                id11(time, name);
                tablesBack.erase(tablesBack.find(name));
            }
            else{
                quOnTables.push(name);
            }
                
        }
        void id4(std::string time, std::string name){
        
            std::cout << time << " 4 " << name << std::endl;

            if (camePeople.find(name) == camePeople.end())
                std::cout <<  id13(time, "ClientUnknown") << std::endl;
            else{
                id12(time, name);
            }
        }

};

class Requests: protected IncomingEvents{
    private:
        std::string time;
        int id;
        std::string person;
        int numT;
    public:
        Requests(std::string valueTime, int numId, std::string namePerson, int numTable=-1){
            time = valueTime;
            id = numId;
            person = namePerson;
            numT = numTable;
        }

        void event(){
            if (id == 1)
                id1(time, person);
            else if (id == 2)
                id2(time, person, numT);
            else if (id == 3)
                id3(time, person);
            else if (id == 4)
                id4(time, person);


            else if (id == 11)
                id11(time, person);
        }
};

int main(int argc, char* argv[]){
    freopen(argv[1], "r", stdin);

    std::string strTimeStart, strTimeEnd;
    
    std::cin >> countPC >> strTimeStart >> strTimeEnd >> priceH;
    timeStart = TimeToInt(strTimeStart); timeEnd = TimeToInt(strTimeEnd);

    for (int i = 0; i < countPC; i++){
        tablesForward[i].second = "";
    }
        
    std::cout << strTimeStart << std::endl;    

    std::string time, name; 
    int id, numT;
    while (std::cin >> time >> id >> name){ // Читаем файлик
        if (id == 2)
            std::cin >> numT;

        Requests request(time, id, name, numT);
        request.event();
    }

    // Закрытие клуба
    while (camePeople.size() > 0){
        std::string name = *camePeople.begin();
        std::cout << strTimeEnd + " 11 " + name << std::endl;
        Requests request(strTimeEnd, 11, name);
        request.event();
    }

    // Финальный счет 
    std::cout << strTimeEnd << std::endl;
    for (auto elem : score)
        std::cout << elem.first+1 << ' ' << elem.second.first << ' ' << TimeToStr(elem.second.second) << std::endl;
    
}