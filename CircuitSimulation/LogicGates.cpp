#include <iostream>
#include <string>
#include <vector>
#include <fstream>
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
	string name;
	bool value;
};

class Components //class used to describe circuit components 
{
public:
	string component_name; //the name of the component 
	LogicGates gate; //the gate used to implement this particular operation
	vector<BoolVar*> inputs; //a vector of inputs (so we can change the size if another input is added/removed)
	BoolVar output; //the output variable

};

void ReadLibrary(vector<LogicGates*>& gates) //function that reads the Lib file
{
	ifstream inputFile("Library.lib"); //reading the file

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
	}
}

void ReadCircuit(vector<LogicGates*>& gates ,vector<Components*>& components, vector<BoolVar*>& inputs)
{
	ifstream inputFile("Circuit.cir"); //reading the circuit file
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
				for (int i = 0; i < gates.size(); i++) //opening the vector gates
				{
					if (gates[i]->component_name == line) //if the component name matches what's in the file
					{
						component->gate = *gates[i]; //add it to the logic gate part of component
					}
				}
				getline(inputFile, line, ','); //move to the next part
				component->output.name = line; //add the output names to component
				getline(inputFile, line, ','); //move to the next part
				for (int i = 0; i < component->gate.inputs; i++) //checking if inputs is repeated or no
				{
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
					
					if (found == true) //if the input exists
					{
						if (i != component->gate.inputs - 1)
						{
							getline(inputFile, line, ','); //skip this and move to the next part of line
						}
						
					}
					else
					{
						BoolVar* input = new BoolVar(); //create a new input
						input->name = line; //puts its name
						input->value = false; //puts its value
						component->inputs.push_back(input); //push back 
						if (i != component->gate.inputs - 1)
						{
							getline(inputFile, line, ','); //go to next part
						}
					}
					
					
				}
				components.push_back(component); //push back the component
			}
		}
	}
	else
	{
		cout << "Unable to open file";
	}


}


int main()
{

	vector<LogicGates*> gates; //create instance of LogicGates
	vector<BoolVar*> inputs; //create instance of BoolVar
	vector<Components*> components; //create instance of Components
	ReadLibrary(gates); //read the library file and write into the gates vector
	ReadCircuit(gates, components, inputs); //read the circuit file and write into components and inputs vectors
//everything else is a test case to display the outputs
	for (int i = 0; i < gates.size(); i++)
	{
		cout << gates[i]->component_name << endl;
		cout << gates[i]->inputs << endl;
		cout << gates[i]->functionality << endl;
		cout << gates[i]->delayps << endl;
		cout << endl;
	}

	for (int i = 0; i < inputs.size(); i++)
	{
		cout << inputs[i]->name << endl;
		cout << inputs[i]->value << endl;
		cout << endl;
	}

	for (int i = 0; i < components.size()-1; i++)
	{
		cout << components[i]->component_name << endl;
		cout << components[i]->gate.component_name << endl;
		cout << components[i]->gate.inputs << endl;
		cout << components[i]->gate.functionality << endl;
		cout << components[i]->gate.delayps << endl;
		cout << components[i]->inputs[0]->name << endl;
		cout << components[i]->output.value << endl;
		cout << components[i]->output.name << endl;
		cout << endl;
	}

}

