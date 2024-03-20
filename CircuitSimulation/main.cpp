
#include "mainwindow.h"
#include <QFileDialog>
#include <QApplication>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <QMessageBox>
#include <stack>
using namespace std;

class LogicGates //Logic Gates class based on the provided description in the project file
{
public:
    string component_name; //name of the component (AND, OR, NOT, NAND, NOR, OR XOR)
    int inputs; //how many inputs it takes
    string functionality; //what the gate does (the boolean expression)
    int delayps; //the delay of the gate expressed in ps
};

class BoolVar //class used to describe the BoolVar (used for both inputs and outputs)
{
public:
    string name; //name of variable
    bool value; //value of variable
    int currtime = 0; //the time of each variable (needed in the simulation)
};

class Components //class used to describe circuit components
{
public:
    string component_name; //the name of the component
    LogicGates gate; //the gate used to implement this particular operation
    vector<BoolVar*> inputs; //a vector of inputs (so we can change the size if another input is added/removed)
    BoolVar* output; //the output variable

};

class stimulus //class for the stimulus file
{
public:
    int time_stamp_ps; //time at which event begins

    BoolVar* input; //the variable that changes (instantiated using pointer of BoolVar)
    bool new_value; //the new value to be assigned to the variable

};

int Precedence(char A) //function that gets the precedence (step in the process of changing the functionality to a valid postfix expression)
{
    if(A == '~') //highest precedence is negation
    {
        return 5; //return the highest number
    }
    else if (A == '&') //& is second highest precedence
    {
        return 4; //return second highest number
    }
    else if (A == '^') //^ is 3rd highest precedence
    {
        return 3; //return 3rd highest number
    }
    else if (A == '|') //| is 4th highest precedence
    {
        return 2; //return 4th highest number
    }
    else if(A == '(') //( is 5th highest precedence
    {
        return 1; //return 5th highest number
    }

}

string fileOptimizer(string text){ //function that optimizes the file and generalizes it by removing spaces(takes in a string)

    string updated; //the string to be returned
    stack<char> s1; //first stack (push string inside it)
    stack <char> s2; //second stack (add every character that isn't a space)
    int n = 0; //varibale for length of text
    char c; //to push in stack
    while(n != text.length()){ //if the text is not done
        c = text[n]; //read the character
        s1.push(c); //push in the first stack
        n++; //go to next character
    }
    while(!s1.empty()){ //while there are still elements in the stack
        if(s1.top() != ' '){ //if the top character is not a space
            s2.push(s1.top()); //add it to second stack
        }
        s1.pop(); //go to next element in stack
    }
//text is now inverted in stack2. We need to bring it back
    while(!s2.empty()){ //while there are still elements
       updated += s2.top(); //write to string
        s2.pop(); //go to next element in stack
    }

    return updated; //now a string without spaces
}

BoolVar* character_to_operator(BoolVar* A, BoolVar* B, char C, Components* component) //function that handles the behaviors of the different boolean functions (using bitwise operators)
{
    BoolVar* Out = new BoolVar(); //the out to be returned
    Out->name = "PlaceHolder :D"; //just a name given to it (not that important but needed to be done to avoid errors in the instantiation)

    switch(C)
    {
    case '&': //if AND
        Out->value = A->value & B->value; //perform AND operation

        break;
    case '|': //if OR
        Out->value = A->value | B->value; //perform OR operation

        break;
    case '~': //if NOT
        Out->value = A->value ^ true; // perform NOT operation

        break;
    case '^': //if XOR
        Out->value = A->value ^ B->value; //perform XOR operation

        break;
    }

    return Out; //return the output
}

string Postfix(Components* component) //function that turns the components to an expression in Postfix
{
    string postfix;
    stack<char> holder;
    for(int i = 0; i<component->gate.functionality.size(); i++)
    {
        if((component->gate.functionality[i] != '&') &&(component->gate.functionality[i] != '|') && (component->gate.functionality[i] != '~')&& (component->gate.functionality[i] != '^')&& (component->gate.functionality[i] != '(')&& (component->gate.functionality[i] != ')'))
        {
            postfix.push_back(component->gate.functionality[i]);
        }
        else if((component->gate.functionality[i] == '&')||(component->gate.functionality[i] == '|')||(component->gate.functionality[i] == '~')||(component->gate.functionality[i] == '^'))
        {
            while (!holder.empty() && ((Precedence(holder.top()) >= Precedence(component->gate.functionality[i]))))
            {
                postfix.push_back(holder.top());
                holder.pop();
            }
            holder.push(component->gate.functionality[i]);
        }
        else if (component->gate.functionality[i] == '(')
        {
            holder.push(component->gate.functionality[i]);
        }
        else if (component->gate.functionality[i] == ')')
        {
            while(holder.top() != '(' && !holder.empty())
            {
                postfix.push_back(holder.top());
                holder.pop();
            }

            holder.pop();
        }
    }

    while(!holder.empty())
    {
        if(holder.top() == '(')
        {
            holder.pop();
        }
        else
        {
            postfix.push_back(holder.top());
            holder.pop();
        }
    }

    return postfix;

}

void postfix_to_bool(Components* component, string postfix, int& time, ofstream& outputFile)
{
    BoolVar* holder1;
    BoolVar* holder2;
    stack<BoolVar*> holderstack;
    char NextChar;
    int index;
    int timecontroller;
    bool oldvalue;

    for(int i = 0; i<postfix.size(); i++)
    {
        if(postfix[i] == 'i')
        {
            NextChar = postfix[++i];
            if(NextChar >= '0' && NextChar <= '9')
            {
                index = NextChar - '0';
                holderstack.push(component->inputs[index-1]);
            }
            else
            {
                throw QMessageBox::critical(nullptr, "error", "Invalid Naming scheme in the .Lib file");
                exit(0);
            }
        }
        else if(postfix[i] == '&' || postfix[i] == '|' || postfix[i] == '^')
        {
            holder2 = holderstack.top();
            holderstack.pop();
            holder1 = holderstack.top();
            holderstack.pop();
            holderstack.push(character_to_operator(holder2, holder1, postfix[i], component));
            time = max(holder1->currtime, holder2->currtime);

        }
        else if(postfix[i] == '~')
        {
            holder2 = holderstack.top();
            holderstack.pop();
            holderstack.push(character_to_operator(holder2,holder2,postfix[i], component));



        }
    }
    oldvalue = component->output->value;
    component->output ->value = holderstack.top()->value;
    component->output->currtime = time + component->gate.delayps;
    if(component->output->value != oldvalue)
    {
        outputFile << component->output->currtime << ", " << component->output->name << ", " << component->output->value << endl;
    }
    holderstack.pop();
}

void ReadLibrary(vector<LogicGates*>& gates, QString path) //function that reads the Lib file
{
    ifstream inputFile(path.toStdString()); //reading the file

    if (inputFile.is_open()) //if successfully opened
    {
        string line;
        while (getline(inputFile, line, ',')) //while there are still lines
        {
            LogicGates* gate = new LogicGates(); //declare a gate
            gate->component_name = line; //gate component is inserted
            getline(inputFile, line, ','); //read the next part of the line
            gate->inputs = stoi(line); //number of inputs is inserted
            getline(inputFile, line, ','); //read the next part of the line
            line = fileOptimizer(line);
            gate->functionality = line; //the functionality is inserted
            getline(inputFile, line, '\n'); //read the next part
            gate->delayps = stoi(line); //add the delay component
            gates.push_back(gate); //push back in gates vectors
        }
        inputFile.close(); //close the file
    }
    else //if file wasn't opened
    {
        cout << "Unable to open file";
        exit(0);
    }
}

void ReadCircuit(vector<LogicGates*>& gates ,vector<Components*>& components, vector<BoolVar*>& inputs, QString path)
{
    ifstream inputFile(path.toStdString()); //reading the circuit file
    string line;
    bool found = false;

    if (inputFile.is_open()) //if file opened
    {
        getline(inputFile, line); //read the line
        if (line == "INPUTS:") //if we're in the INPUTS section
        {
            while (getline(inputFile, line) && line != "COMPONENTS:") //while there are still inputs AND we haven't reached COMPONENTS
            {
                BoolVar* input = new BoolVar(); //create input instance
                input->name = line; //add the name
                input->value = false; //add the value
                inputs.push_back(input); //add to number of inputs vector
            }

        }


        if (line == "COMPONENTS:") //if we're in the components section
        {
            while (getline(inputFile, line, ','))
            {
                Components* component = new Components();
                component->component_name = line; //adding components name
                getline(inputFile, line, ',');
                line = fileOptimizer(line);
                for (int i = 0; i < gates.size(); i++) //opening the vector gates
                {
                    if (gates[i]->component_name == line) //if the component name matches what's in the file
                    {
                        component->gate = *gates[i]; //add it to the logic gate part of component
                    }
                }
                getline(inputFile, line, ','); //move to the next part
                line = fileOptimizer(line);
                component->output = new BoolVar();
                component->output->name = line; //add the output names to component
                component->output->value = false;
                inputs.push_back(component->output);
                for (int i = 0; i < component->gate.inputs; i++) //checking if inputs is repeated or no
                {
                    if(i != component->gate.inputs - 1)
                    {
                        getline(inputFile, line, ','); //move to the next part
                        line = fileOptimizer(line);
                    }
                    else
                    {
                        getline(inputFile, line, '\n');
                        line = fileOptimizer(line);
                    }

                    found = false; //assuming no repetitions
                    for (int j = 0; j < inputs.size(); j++) // looping over the number of inputs
                    {
                        if (inputs[j]->name == line) //if inputs match
                        {
                            component->inputs.push_back(inputs[j]); //push the input
                            found = true; //it has been found!
                            break; //exit for loop
                        }

                    }


                    if (found == false)
                    {
                        BoolVar* input = new BoolVar(); //create a new input
                        line = fileOptimizer(line) ;
                        input->name = line; //puts its name
                        input->value = false; //puts its value
                        component->inputs.push_back(input); //push back

                    }


                }
                components.push_back(component); //push back the component

            }
        }
    }
    else
    {
        cout << "Unable to open file";
        exit(0);
    }

}

void ReadStimulus(vector <stimulus*> &stimuli, vector<BoolVar*>& Inputs, QString path)
{
    ifstream inputFile(path.toStdString()); //reading the file

    if (inputFile.is_open()) //if successfully opened
    {
        string line;
        while (getline(inputFile, line, ',')) //while there are still lines
        {
            stimulus* stimuluss = new stimulus(); //declare a gate
            stimuluss->time_stamp_ps=stoi(line); //gate component is inserted
            getline(inputFile, line, ','); //read the next part of the line
            line=fileOptimizer(line); //number of inputs is inserted
            for(int i = 0; i < Inputs.size(); i++)
            {
                if(line == Inputs[i]->name)
                {
                    stimuluss->input = Inputs[i];
                }
            }
            getline(inputFile,line,'\n');
            stimuluss->new_value=stoi(line); //the functionality is inserted
            stimuli.push_back(stimuluss);
        }
        inputFile.close(); //close the file
    }
    else //if file wasn't opened
    {
        cout << "Unable to open file";
        exit(0);
    }
}


void FileErrorHandling(QString path) //function that handles error
{
    if(path.isEmpty())
    {
        QMessageBox::critical(nullptr, "error", "Empty File"); //if user did not select a file at the beginning
        exit(0);
    }
}

void InputChecker(vector <stimulus*>& stimuli, vector <BoolVar*>& Inputs, int i, ofstream& outputFile, int& time)
{
    for(int j = 0; j<Inputs.size(); j++)
    {

        if(stimuli[i]->input->name == Inputs[j]->name)
        {
            Inputs[j]->value = stimuli[i]->new_value;
            Inputs[j]->currtime = stimuli[i]->time_stamp_ps;
            outputFile << time << ", " << Inputs[j]->name << ", " << Inputs[j]->value << endl;
        }

    }
}

void Simulation(vector <stimulus*>& stimuli, vector <Components*>& Components, vector <BoolVar*>& Inputs, QString filePath4) //simulation function
{
    ofstream outputFile(filePath4.toStdString());
    string postfix;
    int time = 0;
    int c = 0;
    if(!outputFile.is_open())
    {
        QMessageBox::critical(nullptr, "error", "Unable To Open The File");
        exit(0);
    }

    outputFile.clear();
    for (int j = 0; j < Components.size(); j++)
    {
        postfix_to_bool(Components[j], Postfix(Components[j]), time, outputFile);

    }

    for(int i = 0; i<stimuli.size(); i++)
    {
        c = i+1;

        time = stimuli[i]->time_stamp_ps;

        InputChecker(stimuli,Inputs,i, outputFile, time);

        if(c <stimuli.size())
        {
            while(time == stimuli[c]->time_stamp_ps)
            {
                if(c !=stimuli.size())
                {
                    InputChecker(stimuli,Inputs,c, outputFile,time);
                    c++;
                    i = c-1;

                }
            }
        }


        
        for (int j = 0; j < Components.size(); j++)
        {
            postfix_to_bool(Components[j], Postfix(Components[j]), time, outputFile);

        }


    }



   for(int i = 0; i<Components.size(); i++)
   {
       cout << Components[i]->output->name << endl;
       cout << Components[i]->output->value << endl << endl;
   }
}



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMessageBox::information(nullptr, "Information", "Choose a .lib file");
    QString filePath = QFileDialog::getOpenFileName(nullptr, "Select a File", "", "Lib Files (*.lib);;All Files (*)");
    FileErrorHandling(filePath);
    QMessageBox::information(nullptr, "Information", "Choose a .cir file");
    QString filePath2 = QFileDialog::getOpenFileName(nullptr, "Select a File", "", "Circuit Files (*.cir);;All Files (*)");
    FileErrorHandling(filePath2);
    QMessageBox::information(nullptr, "Information", "Choose a .stim file");
    QString filePath3= QFileDialog::getOpenFileName(nullptr, "Select a File", "", "Stim Files (*.stim);;All Files (*)");
    FileErrorHandling(filePath3);
    QMessageBox::information(nullptr, "Information", "Choose a .sim file");
    QString filePath4= QFileDialog::getOpenFileName(nullptr, "Select a File", "", "Sim Files (*.sim);;All Files (*)");
    FileErrorHandling(filePath4);
    vector<LogicGates*> gates; //create instance of LogicGates
    vector<BoolVar*> inputs; //create instance of BoolVar
    vector<Components*> components; //create instance of Components
    vector<stimulus*> stimuli;
    ReadLibrary(gates, filePath); //read the library file and write into the gates vector
    ReadCircuit(gates, components, inputs, filePath2); //read the circuit file and write into components and inputs vectors
    ReadStimulus(stimuli,inputs,filePath3);
    Simulation(stimuli,components,inputs,filePath4);
}
