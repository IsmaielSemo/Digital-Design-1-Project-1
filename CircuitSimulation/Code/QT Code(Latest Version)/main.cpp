
#include "mainwindow.h"
#include <QFileDialog>
#include <QApplication>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <QMessageBox>
#include <stack>
#include <QtCharts/QtCharts>
#include <unistd.h>
#include <QCoreApplication>
#include <QDebug>
#include <windows.h>
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
    bool input = false;
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

void SortedAddition(BoolVar value, vector<BoolVar>& SortedOutput) {
    if (SortedOutput.empty()) {
        SortedOutput.push_back(value);
        return;
    }

    // Case: value's currtime is smaller than the first element in SortedOutput
    if (value.currtime <= SortedOutput.front().currtime) {
        SortedOutput.insert(SortedOutput.begin(), value);
        return;
    }

    // Case: value's currtime is larger than the last element in SortedOutput
    if (value.currtime >= SortedOutput.back().currtime) {
        SortedOutput.push_back(value);
        return;
    }

    // Find the appropriate position to insert value while maintaining sorted order
    for (int i = 0; i < SortedOutput.size(); ++i) {
        if (value.currtime <= SortedOutput[i].currtime) {
            SortedOutput.insert(SortedOutput.begin() + i, value);
            return;
        }
    }
}

void SortedStimuli(stimulus *value, vector<stimulus*>& SortedOutput) {
    if (SortedOutput.empty()) {
        SortedOutput.push_back(value);
        return;
    }

    // Case: value's time_stamp is smaller than the first element in SortedOutput
    if (value->time_stamp_ps <= SortedOutput.front()->time_stamp_ps) {
        SortedOutput.insert(SortedOutput.begin(), value);
        return;
    }

    // Case: value's time_stamp is larger than the last element in SortedOutput
    if (value->time_stamp_ps >= SortedOutput.back()->time_stamp_ps) {
        SortedOutput.push_back(value);
        return;
    }

    // Find the appropriate position to insert value while maintaining sorted order
    for (int i = 0; i < SortedOutput.size(); ++i) {
        if (value->time_stamp_ps <= SortedOutput[i]->time_stamp_ps) {
            SortedOutput.insert(SortedOutput.begin() + i, value);
            return;
        }
    }
}

void Input_Not_Output(BoolVar* output)
{
    string error;
    if(output->input)
    {
        error = output->name + " is an input and can't be a output";
        throw QMessageBox::critical(nullptr, "Error", QString::fromStdString(error) );
        exit(0);
    }
}



int Maximum(int A, int B)
{
    if (A > B)
    {
        return A;
    }
    else if (B > A)
    {
        return B;
    }
    else
    {
        return A;
    }
}

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
    string postfix; //the string that will be returned (in postfix)
    stack<char> holder; //stack used for the bitwise operators and the parentheses
    for(int i = 0; i<component->gate.functionality.size(); i++) //exploring the entirety of the functionality of component
    {
        if((component->gate.functionality[i] != '&') &&(component->gate.functionality[i] != '|') && (component->gate.functionality[i] != '~')&& (component->gate.functionality[i] != '^')&& (component->gate.functionality[i] != '(')&& (component->gate.functionality[i] != ')')) //if character is not a bitwise operator or a parenthesis
        {
            postfix.push_back(component->gate.functionality[i]); //add the character to the end of the postfix string
        }
        else if((component->gate.functionality[i] == '&')||(component->gate.functionality[i] == '|')||(component->gate.functionality[i] == '~')||(component->gate.functionality[i] == '^')) //if the character is a bitwise operator
        {
            while (!holder.empty() && ((Precedence(holder.top()) >= Precedence(component->gate.functionality[i])))) //while there are elements in holder and the precedence of the top element in holder is greater than the current character
            {
                postfix.push_back(holder.top()); //add the top of the holder to the postfix string
                holder.pop(); //explore next element in holder
            }
            holder.push(component->gate.functionality[i]); //push to holder stack
        }
        else if (component->gate.functionality[i] == '(') //if character is open parenthesis
        {
            holder.push(component->gate.functionality[i]); //push the character into the holder stack
        }
        else if (component->gate.functionality[i] == ')') //if character is closed parenthesis
        {
            while(holder.top() != '(' && !holder.empty()) //while the top element is not "(" and the holder isn't empty
            {
                postfix.push_back(holder.top()); //push the top to the end of postfix
                holder.pop(); //explore next top
            }

            holder.pop();
        }
    }

    while(!holder.empty()) //while there are still characters in holder
    {
        if(holder.top() == '(') //if the character at the top is "("
        {
            holder.pop(); //simply pop it and don't add it to the string 
        }
        else
        {
            postfix.push_back(holder.top()); //else push the top of the holder to the end of postfix
            holder.pop(); //explore next element in holder
        }
    }

    return postfix; //return the components functionality now in postfix format as a string
 }

void postfix_to_bool(Components* component, string postfix, int& time, bool firstsim, vector<BoolVar>& SortedOutput) //function that transforms a postfix expression to a boolean one
{//parameters are the components pointer, the postfix string (generated from the previous function), the time (for the simulation) and the output file
    BoolVar* holder1; //BoolVar pointer for input1
    BoolVar* holder2; //BoolVar pointer for input2
    stack<BoolVar*> holderstack; //stack of BoolVar pointer
    char NextChar; 
    int index;
    bool oldvalue;


    for(int i = 0; i<postfix.size(); i++) //while we are still in the postfix expression
    {
        if(postfix[i] != '&' && postfix[i] != '|' && postfix[i] != '^' && postfix[i] != '~') //if the character is "i", meaning it isn't bitwise or parenthesis
        {
            NextChar = postfix[++i]; //get the next character
            if(NextChar >= '0' && NextChar <= '9') //if next character is a number from 0 to 9
            {
                index = NextChar - '0'; //get its index
                holderstack.push(component->inputs[index-1]); //push it to stack
            }
            else
            {
                throw QMessageBox::critical(nullptr, "error", "Invalid Naming scheme in the .Lib file");
                exit(0);
            }
        }
        else if(postfix[i] == '&' || postfix[i] == '|' || postfix[i] == '^') //if the character is a bitwise operator
        {
            holder2 = holderstack.top(); //get the top element and store it in holder2
            holderstack.pop(); //pop it
            holder1 = holderstack.top(); //get the new top and store it in holder1
            holderstack.pop(); //pop it
            holderstack.push(character_to_operator(holder2, holder1, postfix[i], component)); //push in the stack the result of the operation of postfix[i] on holder1 and 2 and store it in component, making use of the character_to_operator function
            time = Maximum(holder1->currtime, holder2->currtime); //the time it takes is the max of both holder1 and holder2
            holderstack.top()->currtime = time;

        }
        else if(postfix[i] == '~') //if the character is the negation operator
        {
            holder2 = holderstack.top(); //get the top in the stack
            time = holder2->currtime;
            holderstack.pop(); //pop it
            holderstack.push(character_to_operator(holder2,holder2,postfix[i], component)); //push in the stack the result of the operation of postfix[i] on holder2 in component
            holderstack.top()->currtime = time;
        }
    }
    oldvalue = component->output->value; //old value of component
    component->output ->value = holderstack.top()->value; //the new value of component (as reached above)
    if(firstsim)
    {
        component->output->currtime = 0;

    }
    else
    {
        component->output->currtime = time + component->gate.delayps;
    }
     //the time it took it reach such output (change of event in variable + gate delay time)
    if(component->output->value != oldvalue) //if the new value is not equal to the old one
    {
        SortedAddition(*component->output, SortedOutput);
    }
    holderstack.pop();
}

void postfix_to_bool_Sequential(Components* component, string postfix, int& time, bool firstsim, vector<BoolVar>& SortedOutput)
{
    BoolVar* holder1; //BoolVar pointer for input1
    BoolVar* holder2; //BoolVar pointer for input2
    stack<BoolVar*> holderstack; //stack of BoolVar pointer
    char NextChar;
    int index;
    bool oldvalue;
    oldvalue = component->output->value;

    for(int i = 0; i<postfix.size(); i++) //while we are still in the postfix expression
    {
        if(postfix[i] != '&' && postfix[i] != '|' && postfix[i] != '^' && postfix[i] != '~') //if the character is "i", meaning it isn't bitwise or parenthesis
        {
            NextChar = postfix[++i]; //get the next character
            if(NextChar >= '0' && NextChar <= '9') //if next character is a number from 0 to 9
            {
                index = NextChar - '0'; //get its index
                holderstack.push(component->inputs[index-1]); //push it to stack
            }
            else
            {
                throw QMessageBox::critical(nullptr, "error", "Invalid Naming scheme in the .Lib file");
                exit(0);
            }
        }
        else if(postfix[i] == '&' || postfix[i] == '|' || postfix[i] == '^') //if the character is a bitwise operator
        {
            holder2 = holderstack.top(); //get the top element and store it in holder2
            holderstack.pop(); //pop it
            holder1 = holderstack.top(); //get the new top and store it in holder1
            holderstack.pop(); //pop it
            if((time == holder1->currtime) || (time == holder2->currtime))
            {
                holderstack.push(character_to_operator(holder2, holder1, postfix[i], component));
                holderstack.top()->currtime = time;
            }
            else
            {
                return;
            }

             //push in the stack the result of the operation of postfix[i] on holder1 and 2 and store it in component, making use of the character_to_operator function


        }
        else if(postfix[i] == '~') //if the character is the negation operator
        {
            holder2 = holderstack.top(); //get the top in the stack
            holderstack.pop(); //pop it
            if(time == holder2->currtime)
            {
                holderstack.push(character_to_operator(holder2,holder2,postfix[i], component));
                holderstack.top()->currtime = time;
            }
            else
            {
                return;
            }

        }
    }
     //old value of component
    component->output ->value = holderstack.top()->value; //the new value of component (as reached above)
    if(firstsim)
    {
        component->output->currtime = 0;

    }
    else if(component->output->value != oldvalue)
    {
        component->output->currtime = time + component->gate.delayps;
        SortedAddition(*component->output, SortedOutput);
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
            line = fileOptimizer(line);
            LogicGates* gate = new LogicGates(); //declare a gate
            gate->component_name = line; //gate component is inserted
            getline(inputFile, line, ','); //read the next part of the line
            gate->inputs = stoi(line); //number of inputs is inserted
            getline(inputFile, line, ','); //read the next part of the line
            line = fileOptimizer(line);
            gate->functionality = line; //the functionality is inserted
            getline(inputFile, line, '\n'); //read the next part
            gate->delayps = stoi(line); //add the delay component
            if(gate->delayps < 0)
            {
                throw QMessageBox::critical(nullptr, "Error", "Negative Delay is not allowed");
                exit(0);
            }
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
    bool found2 = false;
    string info;

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
                input->input = true;
                inputs.push_back(input); //add to number of inputs vector
            }

        }


        if (line == "COMPONENTS:") //if we're in the components section
        {
            while (getline(inputFile, line, ','))
            {
                if(line != "")
                {
                    Components* component = new Components();
                    component->component_name = line; //adding components name
                    getline(inputFile, line, ',');
                    line = fileOptimizer(line);
                    component->gate.component_name = " ";
                    for (int i = 0; i < gates.size(); i++) //opening the vector gates
                    {
                        if (gates[i]->component_name == line) //if the component name matches what's in the file
                        {
                            component->gate = *gates[i]; //add it to the logic gate part of component
                        }
                    }
                    if(component->gate.component_name == " ")
                    {
                        QMessageBox::critical(nullptr, "Error", "Unknown Logic Gate"); //if user did not select a file at the beginning
                        exit(0);
                    }
                    getline(inputFile, line, ','); //move to the next part
                    line = fileOptimizer(line);
                    found2 = false;
                    for(int i = 0; i<inputs.size(); i++)
                    {
                        if(inputs[i]->name == line)
                        {
                            component->output = inputs[i];
                            found2 = true;
                            break;
                        }
                    }


                    if(found2 == false)
                    {
                        component->output = new BoolVar();
                        component->output->name = line; //add the output names to component
                        component->output->value = false;
                        inputs.push_back(component->output);

                    }
                    Input_Not_Output(component->output);

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
                            info = "Component Name: "+ component->component_name + ", " + "Input: " + input->name + ", is either always set to 0, or is part of a Sequential Circuit" ;
                            QMessageBox::information(nullptr, "Information", QString::fromStdString(info));
                            inputs.push_back(input);

                        }


                    }
                    components.push_back(component); //push back the component
                }


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
    string error;

    if (inputFile.is_open()) //if successfully opened
    {
        string line;
        while (getline(inputFile, line, ',')) //while there are still lines
        {
            if(line != "")
            {
                stimulus* stimuluss = new stimulus(); //declare a gate
                stimuluss->time_stamp_ps=stoi(line); //gate component is inserted
                if(stimuluss->time_stamp_ps <0)
                {
                  throw QMessageBox::critical(nullptr, "Error", "Negative Stimulus Time is Invalid");
                }
                getline(inputFile, line, ','); //read the next part of the line
                line=fileOptimizer(line);
                stimuluss->input = new BoolVar();
                stimuluss->input->name = " ";
                for(int i = 0; i < Inputs.size(); i++) //number of inputs is inserted
                {
                    if(line == Inputs[i]->name)
                    {
                        stimuluss->input = Inputs[i];
                    }
                }
                if(stimuluss->input->name == " ")
                {
                    throw QMessageBox::critical(nullptr, "Error", "Unknown Input Detected in Stimuli File");
                }
                else if(stimuluss->input->input == false)
                {
                    throw QMessageBox::critical(nullptr, "Error", QString::fromStdString(stimuluss->input->name) + " is a Wire not an Input");
                }
                getline(inputFile,line,'\n');
                stimuluss->new_value=stoi(line); //the functionality is inserted
                if((stimuluss->new_value != 1) && (stimuluss->new_value != 0))
                {
                    throw QMessageBox::critical(nullptr, "Error", QString::fromStdString(to_string(stimuluss->time_stamp_ps)) + ", " + QString::fromStdString(stimuluss->input->name) + " has an Illegal Value which is:" + QString::fromStdString(to_string(stimuluss->new_value)));
                }
                if(stimuluss->time_stamp_ps == 0 && stimuluss->new_value == 0)
                {
                    error = "Stimuli Removed: " + to_string(stimuluss->time_stamp_ps) + ", " + stimuluss->input->name + ", " + to_string(stimuluss->new_value) + " is Redundant";
                    QMessageBox::information(nullptr, "Information", QString::fromStdString(error));
                }
                else if(stimuluss->time_stamp_ps == 0 && stimuluss->new_value == 1)
                {
                   stimuluss->input->value = stimuluss->new_value;
                   SortedStimuli(stimuluss,stimuli);
                }
                else
                {
                   SortedStimuli(stimuluss,stimuli);
                }

            }

        }
        inputFile.close(); //close the file
    }
    else //if file wasn't opened
    {
        cout << "Unable to open file";
        exit(0);
    }
    for(int i = 0; i < stimuli.size();i++)
        cout << stimuli[i]->input->name << " " << stimuli[i]->time_stamp_ps << endl;
}


void FileErrorHandling(QString path) //function that handles error
{
    if(path.isEmpty())
    {
        QMessageBox::critical(nullptr, "error", "Empty File"); //if user did not select a file at the beginning
        exit(0);
    }
}

void InputChecker(vector <stimulus*>& stimuli, vector <BoolVar*>& Inputs, int i, int& time, vector<BoolVar>& SortedOutput) //function that updates the input parameters (value and time) based on the information in the stimuli file
{
    stimuli[i]->input->value = stimuli[i]->new_value;
    stimuli[i]->input->currtime = stimuli[i]->time_stamp_ps;
    SortedAddition(*stimuli[i]->input, SortedOutput);
}



bool HandleRedundantStim(vector <stimulus*>& stimuli,int x)
{
    if(stimuli[x]->new_value == stimuli[x]->input->value)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Simulation(vector <stimulus*>& stimuli, vector <Components*>& Components, vector <BoolVar*>& Inputs, vector<BoolVar>& SortedOutput) //simulation function
{

    string postfix; //string to be used for postfix
    int time = 0; //time of simulation
    int c = 0;
    bool FirstSim = true;
    auto it = stimuli.begin();
    string error;



    for (int j = 0; j < Components.size(); j++)
    {
        postfix_to_bool(Components[j], Postfix(Components[j]), time, FirstSim, SortedOutput); //will generate a boolean expression given components, a postfix expression, and the time (outputs to file)

    }

    for (int i = 0; i <Inputs.size(); i++)
    {
        if((Inputs[i]->value == 0) || (Inputs[i]->input== true && Inputs[i]->currtime == 0 && Inputs[i]->value == 1))
        {
            SortedAddition(*Inputs[i], SortedOutput);
        }
    }


    FirstSim = false;

    for(int i = 0; i<stimuli.size(); i++)
    {

        if(HandleRedundantStim(stimuli, i))
        {
            error = "Removed " + stimuli[i]->input->name + ", " + to_string(stimuli[i]->time_stamp_ps) + ", " +to_string(stimuli[i]->new_value) + ": It was Redundant";
            QMessageBox::information(nullptr, "Information", QString::fromStdString(error));
            stimuli.erase(it);
            i--;
            continue;
        }

        c = i+1; //check the following input

        time = stimuli[i]->time_stamp_ps; //get the time of the input we are at

        InputChecker(stimuli,Inputs,i, time, SortedOutput); //apply the changes to the input through the event

        if(c <stimuli.size()) //if the next input is still within the stimuli
        {
            while(time == stimuli[c]->time_stamp_ps) //while the time of input we're at is the same as the one after
            {
                if(c !=stimuli.size())
                {
                    InputChecker(stimuli,Inputs,c,time, SortedOutput); //apply the changes to the next input through the event
                    c++; //go to the next character
                    i = c-1; //change the i accordingly (accelerating the for loop)

                }
            }
        }


        
        for (int j = 0; j < Components.size(); j++)
        {
            postfix_to_bool(Components[j], Postfix(Components[j]), time, FirstSim, SortedOutput); //get the boolean expression of the now changed inputs

        }

        it++;
    }



   for(int i = 0; i<Components.size(); i++) //displaying all the components (output name and value)
   {
       cout << Components[i]->output->name << endl;
       cout << Components[i]->output->value << endl << endl;
   }
}

void SequentialSimulator(vector <stimulus*>& stimuli, vector <Components*>& Components, vector <BoolVar*>& Inputs, vector<BoolVar>& SortedOutput)
{
   string postfix; //string to be used for postfix
   bool FirstSim = true;
   auto it = stimuli.begin();
   int initial_time = 0;
   string error;
   int stimcount = 0;



   for (int j = 0; j < Components.size(); j++)
   {
       postfix_to_bool(Components[j], Postfix(Components[j]), initial_time , FirstSim, SortedOutput); //will generate a boolean expression given components, a postfix expression, and the time (outputs to file)

   }

   FirstSim = false;

   for (int i = 0; i <Inputs.size(); i++)
   {
       if(Inputs[i]->value == 0 || (Inputs[i]->input== true && Inputs[i]->currtime == 0 && Inputs[i]->value == 1))
       {
            SortedAddition(*Inputs[i], SortedOutput);
       }
   }

   for(int i=0; i <1000000; i++)
   {
       if(stimcount != stimuli.size())
       {
        while((stimuli[stimcount]->time_stamp_ps == i))
            {
                if(HandleRedundantStim(stimuli, stimcount))
                {
                    error = "Removed " + stimuli[stimcount]->input->name + ", " + to_string(stimuli[stimcount]->time_stamp_ps) + ", " +to_string(stimuli[stimcount]->new_value) + ": It was Redundant";
                    QMessageBox::information(nullptr, "Information", QString::fromStdString(error));
                    stimuli.erase(it);

                }
                else
                {
                    InputChecker(stimuli,Inputs,stimcount, i, SortedOutput);
                    stimcount++;
                    it++;
                    if(stimcount == stimuli.size())
                    {
                        break;
                    }
                 }
            }
       }

       for (int j = 0; j < Components.size(); j++)
       {
            postfix_to_bool_Sequential(Components[j], Postfix(Components[j]),i, FirstSim, SortedOutput); //get the boolean expression of the now changed inputs

       }


   }
}

bool SequentialDetector(vector <Components*>& Components)
{
   for(int i = 0; i<Components.size()-1; i++)
   {
       for(int j = i+1; j<Components.size(); j++)
       {
            for(int k = 0; k<Components[i]->inputs.size(); k++)
            {
                for(int l=0; l<Components[j]->inputs.size(); l++)
                {
                    if((Components[i]->output->name == Components[j]->inputs[l]->name) && (Components[i]->inputs[k]->name == Components[j]->output->name))
                    {
                        return true;
                    }
                }
            }
       }
   }

   return false;
}

void PrintInSim(QString filePath4, vector<BoolVar>& SortedOutput)
{
   ofstream outputFile(filePath4.toStdString()); //the file that we will write to
   if(!outputFile.is_open()) //error handling if the file did not open
   {
       QMessageBox::critical(nullptr, "error", "Unable To Open The File");
       exit(0);
   }
   outputFile.clear();

   for(int i=0; i<SortedOutput.size(); i++)
   {
       if(SortedOutput[i].currtime != 0)
       {
            outputFile << SortedOutput[i].currtime << ", " << SortedOutput[i].name << ", " << SortedOutput[i].value << endl;
       }
   }

   outputFile.close();
}

int MaxValueGrabber(vector<BoolVar>& SortedOutput)
{
   int max = SortedOutput[0].currtime;
   for(int i =1; i< SortedOutput.size(); i++)
   {
       if(SortedOutput[i].currtime > max)
       {
            max = SortedOutput[i].currtime +1000;
       }
   }

   return max;
}

void DrawTimeGraphs(vector<BoolVar>& SortedOutput, bool SequentialFlag, int inputsize)
{
   QChartView* chartView = new QChartView();
   QChart*  chart = new QChart();
   vector<QLineSeries*> lines;
   QValueAxis* axisX = new QValueAxis;
   QValueAxis* axisY = new QValueAxis;
   vector<int> lastpoint;
   int max = MaxValueGrabber(SortedOutput);

   // Set range and labels for the X axis
   axisX->setRange(0, max);
   axisX->setLabelFormat("%.0f"); // Format for axis labels (optional)
   axisX->setTitleText("Time/ps"); // Axis title
   //axisX->applyNiceNumbers();
   axisX->setLabelsColor(Qt::white);
   axisX->setTitleBrush(Qt::white);

   // Set range and labels for the Y axis
   if(SequentialFlag)
   {
       axisY->setRange(0, 3*float(inputsize));
   }
   else
   {
        axisY->setRange(0, 1.5*float(SortedOutput.size()));
   }

   axisY->setLabelsVisible(false);
   axisY->setLabelFormat("%.0f"); // Format for axis labels (optional)
   axisY->setTitleText("Output"); // Axis title
   axisY->setLabelsColor(Qt::white);
   axisY->setTitleBrush(Qt::white);
   chart->setLocalizeNumbers(true);

   // Add axes to the chart
   chart->setTitleBrush(Qt::white);
   chart->setTheme(QChart::ChartTheme::ChartThemeBlueCerulean);
   chart->addAxis(axisX, Qt::AlignBottom);
   chart->addAxis(axisY, Qt::AlignLeft);

   QPen* Pen = new QPen();
   Pen->setWidth(6);

   for(int i = 0; i<SortedOutput.size(); i++)
   {
       if(SortedOutput[i].currtime==0)
       {
            lines.push_back(new QLineSeries());
            lines[i]->setPen(*Pen);
            lines[i]->setName(QString::fromStdString(SortedOutput[i].name));
            lines[i]->setColor(QColor::fromRgb(rand()%256, rand()%256, rand()%256));
            if(SequentialFlag)
            {
                lines[i]->append(2*inputsize,static_cast<int>(SortedOutput[i].value)+2*(i+1));
                lines[i]->append(SortedOutput[i].currtime,static_cast<int>(SortedOutput[i].value)+2*(i+1));
                lines[i]->append(max, static_cast<int>(SortedOutput[i].value)+2*(i+1));
            }

            else
            {
                lines[i]->append(2*SortedOutput.size(),static_cast<int>(SortedOutput[i].value)+2*(i+1));
                lines[i]->append(SortedOutput[i].currtime,static_cast<int>(SortedOutput[i].value)+2*(i+1));
                lines[i]->append(max, static_cast<int>(SortedOutput[i].value)+2*(i+1));
            }

       }
       else
       {
            for(int j =0; j<lines.size(); j++)
            {
                if(lines[j]->name().toStdString() == SortedOutput[i].name)
                {
                    lines[j]->remove(max,static_cast<int>(!SortedOutput[i].value)+2*(j+1));
                    lines[j]->append(SortedOutput[i].currtime,static_cast<int>(!SortedOutput[i].value)+2*(j+1));
                    lines[j]->append(SortedOutput[i].currtime,static_cast<int>(SortedOutput[i].value)+2*(j+1));
                    lines[j]->append(max,static_cast<int>(SortedOutput[i].value)+2*(j+1));

                }

            }
       }

   }

   for(int i = 0; i<lines.size(); i++)
   {
       chart->addSeries(lines[i]);
       lines[i]->attachAxis(axisX);
       lines[i]->attachAxis(axisY);
   }

   chart->setBackgroundBrush(Qt::black);
   chartView->setChart(chart);
   chartView->setRubberBand(QChartView::HorizontalRubberBand);
   chartView->show();

}



int main(int argc, char *argv[])
{

   QApplication a(argc, argv);
   QString filePath;
   QString filePath2;
   QString filePath3;
   QString filePath4;


   if(!isatty(STDIN_FILENO))
   {
       QMessageBox::information(nullptr, "Information", "Running From Terminal");
       QStringList arguments = a.arguments();
       int i = 0;
       for(const QString& arg : arguments)
       {
            if(i ==1)
            {
                filePath = arg;
            }
            else if(i ==2)
            {
               filePath2 = arg;
            }
            else if(i==3)
            {
               filePath3 = arg;
            }
            else if(i == 4)
            {
               filePath4 = arg;
            }
            i++;
       }

   }
   else
   {
       QMessageBox::information(nullptr, "Information", "Choose a .lib file");
       filePath = QFileDialog::getOpenFileName(nullptr, "Select a File", "", "Lib Files (*.lib);;All Files (*)");
       FileErrorHandling(filePath);
       QMessageBox::information(nullptr, "Information", "Choose a .cir file");
       filePath2 = QFileDialog::getOpenFileName(nullptr, "Select a File", "", "Circuit Files (*.cir);;All Files (*)");
       FileErrorHandling(filePath2);
       QMessageBox::information(nullptr, "Information", "Choose a .stim file");
       filePath3= QFileDialog::getOpenFileName(nullptr, "Select a File", "", "Stim Files (*.stim);;All Files (*)");
       FileErrorHandling(filePath3);
       QMessageBox::information(nullptr, "Information", "Choose a .sim file");
       filePath4= QFileDialog::getOpenFileName(nullptr, "Select a File", "", "Sim Files (*.sim);;All Files (*)");
       FileErrorHandling(filePath4);
   }

    vector<LogicGates*> gates; //create instance of LogicGates
    vector<BoolVar*> inputs; //create instance of BoolVar
    vector<Components*> components; //create instance of Components
    vector<stimulus*> stimuli;
    vector<BoolVar> SortedOutput;
    bool Sequentialflag;
    ReadLibrary(gates, filePath); //read the library file and write into the gates vector
    ReadCircuit(gates, components, inputs, filePath2); //read the circuit file and write into components and inputs vectors
    ReadStimulus(stimuli,inputs,filePath3);
    Sequentialflag = SequentialDetector(components);
    if(Sequentialflag)
    {
       SequentialSimulator(stimuli,components,inputs, SortedOutput);
       DrawTimeGraphs(SortedOutput, Sequentialflag, inputs.size());
    }
    else
    {
       Simulation(stimuli,components,inputs, SortedOutput);
       DrawTimeGraphs(SortedOutput, Sequentialflag, inputs.size());

    }

    PrintInSim(filePath4, SortedOutput);

    for(int i= 0; i<SortedOutput.size(); i++)
    {
       cout << SortedOutput[i].currtime << ", " << SortedOutput[i].name << ", " << SortedOutput[i].value << endl;
    }

    return a.exec();
}
