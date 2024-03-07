#include <iostream>
#include <string>
#include <vector>
#include <fstream>
using namespace std;

class LogicGates
{
public:
	string component_name;
	int inputs;
	string functionality;
	int delayps;
};

class Inputs
{
public:
	string name;
	bool value;
};

class Components
{
public:
	string component_name;
	LogicGates gate;
	vector<Inputs*> inputs;
	Inputs output;

};

void ReadLibrary(vector<LogicGates*>& gates)
{
	ifstream inputFile("Libary.lib");

	if (inputFile.is_open())
	{
		string line;
		while (getline(inputFile, line, ','))
		{
			LogicGates* gate = new LogicGates();
			gate->component_name = line;
			getline(inputFile, line, ',');
			gate->inputs = stoi(line);
			getline(inputFile, line, ',');
			gate->functionality = line;
			getline(inputFile, line, '\n');
			gate->delayps = stoi(line);
			gates.push_back(gate);
		}
		inputFile.close();
	}
	else
	{
		cout << "Unable to open file";
	}
}

void ReadCircuit(vector<LogicGates*>& gates ,vector<Components*>& components, vector<Inputs*>& inputs)
{
	ifstream inputFile("Circuit.cir");
	string line;
	bool found = false;

	if (inputFile.is_open())
	{
		getline(inputFile, line);
		if (line == "INPUTS:")
		{
			while (getline(inputFile, line) && line != "COMPONENTS:")
			{
				Inputs* input = new Inputs();
				input->name = line;
				input->value = false;
				inputs.push_back(input);
			}

		}
		
			
		if (line == "COMPONENTS:")
		{
			while (getline(inputFile, line, ','))
			{
				Components* component = new Components();
				component->component_name = line;
				getline(inputFile, line, ',');
				for (int i = 0; i < gates.size(); i++)
				{
					if (gates[i]->component_name == line)
					{
						component->gate = *gates[i];
					}
				}
				getline(inputFile, line, ',');
				component->output.name = line;
				getline(inputFile, line, ',');
				for (int i = 0; i < component->gate.inputs; i++)
				{
					found = false;
					for (int j = 0; j < inputs.size(); j++)
					{
						if (inputs[j]->name == line)
						{
							component->inputs.push_back(inputs[j]);
							found = true;
							break;
						}
						
					}
					
					if (found == true)
					{
						if (i != component->gate.inputs - 1)
						{
							getline(inputFile, line, ',');
						}
						
					}
					else
					{
						Inputs* input = new Inputs();
						input->name = line;
						input->value = false;
						component->inputs.push_back(input);
						if (i != component->gate.inputs - 1)
						{
							getline(inputFile, line, ',');
						}
					}
					
					
				}
				components.push_back(component);
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

	vector<LogicGates*> gates;
	vector<Inputs*> inputs;
	vector<Components*> components;
	ReadLibrary(gates);
	ReadCircuit(gates, components, inputs);

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

/*

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

		for (int i = 0; i < components.size() -1 ; i++)
		{
			cout << components[i]->component_name << endl;
			cout << components[i]->gate.component_name << endl;
			cout << components[i]->gate.inputs << endl;
			cout << components[i]->gate.functionality << endl;
			cout << components[i]->gate.delayps << endl;
			cout << components[i]->output.value << endl;
			cout << components[i]->output.name << endl;
			cout << endl;
		}

		*/