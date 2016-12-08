//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 


#include <AVLTree.h>
#include <Headers.h>

Define_Module(Router);

std::string Router::routTab0[][4]= {
        {PC0,PC0,"0","29"},
        {PC1,R10,"1","26"},
        {PC2,R10,"1","24"},
};
std::string Router::routTab1[][4]= {
        {PC0,R01,"0","29"},
        {PC1,R21,"1","26"},
        {PC2,R31,"2","24"},
};
std::string Router::routTab2[][4]= {
        {PC0,R12,"1","29"},
        {PC1,PC1,"0","26"},
        {PC2,R12,"1","24"},
};
std::string Router::routTab3[][4]= {
        {PC0,R13,"1","29"},
        {PC1,R13,"1","26"},
        {PC2,PC2,"0","24"},
};

Router::Router()
{
    ttmsg = nullptr;
    selfmsg = nullptr;
}

Router::~Router()
{
    cancelAndDelete(selfmsg);
    if (ttmsg)
        delete ttmsg;
}

void Router::initialize()
{
    selfmsg = new cMessage("event");

    queueCountStats.setName("queueCountStats");
    //queueCountStats.setRangeAutoUpper(0, 10, 1.5);
    queueCountVector.setName("queueCounter");
    msgResponseTimeVector.setName("response time");

    simulationMode = par("SIM_MODE");

    std::string a = par("address0");
    const char * a1 = a.c_str();
    ipAddress0.set(a1); WATCH(ipAddress0);
    std::string b = par("address1");
    const char * a2 = a.c_str();
    ipAddress1.set(a2); WATCH(ipAddress1);
    std::string c = par("address2");
    const char * a3 = a.c_str();
    ipAddress2.set(a3); WATCH(ipAddress2);

    std::string name = getName();
    const char * routerName = name.c_str();
    if(strcmp(routerName,"Router0")==0)
    {
        for(int i=0; i<3;i++)
        {
            std::string empty = "0.0.0.0";
            node tmp( routTab0[i][0],
                    empty,
                    routTab0[i][1],
                    routTab0[i][2],
                    atoi((routTab0[i][3]).c_str()));

            avlTree.AddNode(tmp);
        }
    }
    else if(strcmp(routerName,"Router1")==0)
    {
        for(int i=0; i<3;i++)
        {
            std::string empty = "0.0.0.0";
            node tmp( routTab1[i][0],
                    empty,
                    routTab1[i][1],
                    routTab1[i][2],
                    atoi((routTab0[i][3]).c_str()));

            avlTree.AddNode(tmp);
        }
    }
    else if(strcmp(routerName,"Router2")==0)
    {
        for(int i=0; i<3;i++)
        {
            std::string empty = "0.0.0.0";
            node tmp( routTab2[i][0],
                    empty,
                    routTab2[i][1],
                    routTab2[i][2],
                    atoi((routTab0[i][3]).c_str()));

            avlTree.AddNode(tmp);
        }
    }
    else if(strcmp(routerName,"Router3")==0)
    {
        for(int i=0; i<3;i++)
        {
            std::string empty = "0.0.0.0";
            node tmp( routTab3[i][0],
                    empty,
                    routTab3[i][1],
                    routTab3[i][2],
                    atoi((routTab0[i][3]).c_str()));

            avlTree.AddNode(tmp);
        }
    }
}

void Router::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage())
    {
        if(ttmsg != nullptr)
        {
            IPAddress dst = ttmsg->getDstAddress();
            simtime_t handlingTime = simTime() - ttmsg->getArrivalTime();

            node* n = avlTree.FindProperNode(dst);
            int p = atoi((n->gate).c_str());

            send(ttmsg, "port$o", p);

            std::string name = getName();
            const char * routerName = name.c_str();
            if(strcmp(routerName,"Router1")==0)
            {
                queueCountVector.record(queue.getLength());
                queueCountStats.collect(queue.getLength());
                msgResponseTimeVector.collect(handlingTime);
            }
        }
        cancelEvent(selfmsg);
        if(!queue.isEmpty())
        {
            ttmsg = check_and_cast<ExtMessage *>(queue.pop());
            if(strcmp(simulationMode,"TCAM")==0)
            {
                scheduleAt(simTime()+2.2e-009, selfmsg);
            }
            else
            {
                double nTime = normal(37.44e-009,20.26e-009);
                scheduleAt(simTime()+nTime, selfmsg);
            }
        }
    }
    else
    {
        ExtMessage *arrmsg = check_and_cast<ExtMessage *>(msg);
        if(queue.isEmpty() && !selfmsg->isScheduled())
        {
            ttmsg = arrmsg;
            if(strcmp(simulationMode,"TCAM")==0)
            {
                scheduleAt(simTime()+2.2e-009, selfmsg);
            }
            else
            {
                double nTime = normal(37.44e-009,20.26e-009);
                scheduleAt(simTime()+nTime, selfmsg);
            }
        }
        else
        {
            queue.insert(arrmsg);
        }
        arrmsg = nullptr;
     }
}

void Router::finish()
{
    // This function is called by OMNeT++ at the end of the simulation.
    EV << "Queue count, min:    " << queueCountStats.getMin() << endl;
    EV << "Queue count, max:    " << queueCountStats.getMax() << endl;
    EV << "Queue count, mean:   " << queueCountStats.getMean() << endl;
    EV << "Queue count, stddev: " << queueCountStats.getStddev() << endl;
    EV << "Msg response, min:    " << msgResponseTimeVector.getMin() << endl;
    EV << "Msg response, max:    " << msgResponseTimeVector.getMax() << endl;
    EV << "Msg response, mean:   " << msgResponseTimeVector.getMean() << endl;
    EV << "Msg response, stddev: " << msgResponseTimeVector.getStddev() << endl;

    queueCountStats.recordAs("queue count");
    msgResponseTimeVector.recordAs("hanling msg time");
}
