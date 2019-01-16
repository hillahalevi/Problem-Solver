#include <utility>

#ifndef HNJ_2NDROAD_MYCLIENTHANDLER_H
#define HNJ_2NDROAD_MYCLIENTHANDLER_H

#include <istream>
#include "ClientHandler.h"
#include "string.h"
#include "../problem_solve/Solver.h"
#include "../cache/CacheManager.h"
#include "AServer.h"
#include "../search/MatrixSearchProblem.h"

#define END_MATRIX "end"

namespace server_side {



    template<class Problem, class Solution>
    class MyClientHandler : public ClientHandler {

        Solver<Problem, Solution> *solver;
        CacheManager *cacheManager;

    public:
        MyClientHandler(Solver<Problem, Solution> *solverToSet, CacheManager *cacheManagerToSet) {
            this->solver = solverToSet;
            this->cacheManager = cacheManagerToSet;
        }


        void getSingleMessage(int socketId, char **buffer) {

            int n;

            bzero(*buffer, BUFFER_SIZE);
            //read bytes
            n = (int) read(socketId, *buffer, (BUFFER_SIZE - sizeof(char)));

            //empty packet - lost connection
            if (n <= ZERO) {
                throw "client disconnected";
            }

        }

        void handleClient(int socketId) override {
            int n;
            char response[BUFFER_SIZE];
            if (socketId == -1) {
                return; //nothing to listen to
            }

            Matrix* matrix;
            /* If connection is established then start communicating */
            while (true) {
                try {

                        matrix = new Matrix();
                        MatrixSearchProblem m = getSingleSearchRequest(socketId, matrix);

                        string problem = m.problemToString();
                        Solution solution;
                        if (cacheManager->isSolution(problem)) {
                            solution = cacheManager->getSolution(problem);
                        } else {

                            solution = solver->solve(m);
                            cacheManager->saveSolution(problem, solution);
                        }
                        delete matrix;
                        std::ostringstream stream;
                        stream << solution;

                        string solutionStr = stream.str();


                        cout << "solution: " << endl;
                        cout << solutionStr << endl;
                        //write response to client - check this please
                        int resultCode;
                        size_t len = solutionStr.length();
                        strcpy(response, solutionStr.c_str());


                        resultCode = (int) send(socketId, response, len, 0);


                        //check message sent
                        if (resultCode < ZERO) {
                            cout << "ERROR writing to socket" << endl;
                        }

                } catch (const char *ex) {

                        return;
                    }
            }
        }

        MatrixSearchProblem getSingleSearchRequest(int socketId, Matrix *matrix) {
            //get whole problem
            MatrixSearchProblem problem;
            vector<string> request = getRequest(socketId);

            //get destination point
            POINT dst = getPoint(request.back());

            //remove destination point from problem
            request.pop_back();

            //get source point
            POINT src = getPoint(request.back());

            problem.setStartEnd(src, dst);

            //remove source point
            request.pop_back();

            problem.setMatrix(getMatrix(request, matrix));
            return problem;

        }

        vector<string> getRequest(int socketId) {
            bool isEndRequest = false;
            char buffer[BUFFER_SIZE];
            char *current = buffer;
            vector<string> request;
            //read matrix data until the end massage
            while (!isEndRequest) {
                getSingleMessage(socketId, &current);

                vector<string> splitedByEnd = Utils::splitbyEndl(buffer);

                for (const string &line:splitedByEnd) {
                    if (line == END_MATRIX) {
                        isEndRequest = true;
                        break;
                    }
                    request.push_back(line);
                }

            }
            return request;

        }

        POINT getPoint(string data) {
            vector<string> info = Utils::split(move(data), ',');
            POINT point;
            point.x = atoi(info[0].data());
            point.y = atoi(info[1].data());

            return point;
        }

        ISearchable<int, POINT> *getMatrix(vector<string> info, Matrix *mat) {
            for (string line:info) {
                mat->addRow(line);
            }

            return mat;
        }


        ~MyClientHandler() {
            solver = nullptr;
            cacheManager = nullptr;
        }

    };
}
#endif //HNJ_2NDROAD_MYCLIENTHANDLER_H