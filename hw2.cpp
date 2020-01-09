// hw2.cpp : ���ɮץ]�t 'main' �禡�C�{���|��ӳB�}�l����ε�������C
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

// From HW1
struct Register {
	string name;
	float value;
};
struct Instruction {
	string Type;
	Register D_Reg; // Destination Register
	Register InputA_Reg;
	Register InputB_Reg;
	int immed; // immediate number
}; 
struct Instuction_History {
	string text;
	int issue_cycle;
	int execu_cycle;
	int wb_cycle;
};
vector<Instuction_History> Inst_history;
vector<Instruction>Inst;
void txt_file_string_cut(string, int); // For txt file data reading
void Instruction_Extract_Data_Saving(string, int, int); // For txt file data saving 

// For HW2 Only // Tomasulo's law Structure
int total_register_number = 0;
struct Register2 {
	string name;
	string replace_name;
};
struct RS_Line{
	string NAME;
	bool busy;                        
	char operation; // +,-,*,/
	float former; 
	float later;
	string former_s;
	string later_s;
	bool dispatch;
	bool execution;
	int execu_priority;
};
struct RS_Buffer{
	bool busy;
	string RS_name;
	string register_name; // �n���Τ���?
	char operation;
	float former;
	float later;
	int finish_cycle;
};
struct Inst_cycle {
	char operation;
	int cycle_number;
};
vector<Register> RF;
vector<Register2> RAT;
vector<RS_Line> RS; // ADD(0~2)+MUL(3&4)
vector<Inst_cycle> Cycle;
vector<int> ready_set;
RS_Buffer Buffer_Add;
RS_Buffer Buffer_Mul;
void Initialization();
void Run_Tomasulo();
void Tomasulo_Check_Buffer_Status(RS_Buffer &, bool, int);
void Tomasulo_Issue(int, int);
void Tomasulo_Issue_Check_RAT(int, string, int , int);
void Tomasulo_Dispatch(bool, int); 
void Tomasulo_Dispatch_Scan_RS(bool, int);
void Tomasulo_Dispatch_Buffer_Assign(RS_Buffer &, int, int);
void ready_set_sorting();
void Cycle_print(int);

int main(){
	int inst_counter = 0;
	string content;
	ifstream myfile("ASSEMBLY_HW2.txt"); // ifstream myfile("ASSEMBLY_Test.txt");
	if (myfile.is_open())
		while (getline(myfile, content)) // �i�H�@��Ū�@��
		{
			Inst_history.push_back({ content, 0, 0, 0 });
			txt_file_string_cut(content, inst_counter); // �ǤJ�r�ꥻ��P�r�ꪺ�Ǽ�
			inst_counter++;
		}

	// cout << "total register number" << total_register_number << endl;

	Initialization();
	Run_Tomasulo();
	// Cycle_print(0);
	

	// For debug
	//for (int i = 0; i < inst_counter; i++)
	//{
	//	cout << Inst_history[i] << "\t"
	//		<< Inst[i].Type << " "
	//		<< Inst[i].D_Reg.name << "+" << Inst[i].D_Reg.value << " "
	//		<< Inst[i].InputA_Reg.name << "+" << Inst[i].InputA_Reg.value << " "
	//		<< Inst[i].InputB_Reg.name << "+" << Inst[i].InputB_Reg.value << " "
	//		<< Inst[i].immed << endl;
	//}
	system("pause");
}
void txt_file_string_cut(string content, int inst_n) // �i��r��B�z
{
	Instruction inst = { "", {"", -1}, {"", -1}, {"", -1}, NULL };
	Inst.push_back(inst);
	string extract = "";

	// �I�X�@�q��r
	while (true)
	{
		int start = content.find_first_not_of(" \t,:"); //cout << start << "\t";
		int end = content.find_first_of(" \t,:\n"); //cout << end << "\t";
		extract = content.substr(0, end);
		Instruction_Extract_Data_Saving(extract, inst_n, content.length()); // ����X�Ӫ��F���iData_Saving�P�_�r�ꪺ�������x�s
		if (end == -1)
			return;

		//��r�갵��z
		content = content.substr(end, content.length() - extract.length());
		int split_n = content.find_first_not_of("\t, :");
		if (split_n == string::npos) // �p�G�w�g��r����ڴN���X
			return;
		content = content.substr(split_n, content.length() - split_n + 1);
		//cout << "." << content << endl;
	}
}
void Instruction_Extract_Data_Saving(string s, int inst, int rest_n)
{
	stringstream geek(s); // ���Ximmediate
	int x;
	if (geek >> x)
	{
		Inst[inst].immed = x;
		return;
	}

	if (s.find("F") != string::npos)
	{
		string s_value = s.substr(1, s.length() - 1);
		int int_s_value = stoi(s_value);
		if (total_register_number < int_s_value)
			total_register_number = int_s_value;

		if (Inst[inst].D_Reg.value == -1) // �s�iDestination Register
		{
			Inst[inst].D_Reg.name = s;
			Inst[inst].D_Reg.value = int_s_value;
			return;
		}
		if (Inst[inst].InputA_Reg.value == -1) 
		{
			Inst[inst].InputA_Reg.name = s;
			Inst[inst].InputA_Reg.value = int_s_value;
			return;
		}

		Inst[inst].InputB_Reg.name = s;
		Inst[inst].InputB_Reg.value = int_s_value;
		return;
	}
	Inst[inst].Type = s;
	return;
}
void Initialization()
{
	// RF and RAT
	string temp = "F";
	for (int i = 0; i < total_register_number; i++)
	{
		temp = "F" + to_string(i + 1);
		RF.push_back({ temp, (float)(i*2) });
		RAT.push_back({ temp, "" });
	}
	
	// ALU and Buffer
	RS_Line temp_alu = { "", false, '.', NULL, NULL, "", "", false, false, 0};
	for (int a = 0; a < 3; a++){
		temp_alu.NAME = "ADD" + to_string(a+1);
		RS.push_back(temp_alu);
	}
	for (int b = 0; b < 2; b++){
		temp_alu.NAME = "MUL" + to_string(b+1);
		RS.push_back(temp_alu);
	}
	Buffer_Add = { false, "", "", '.', -1, -1, -1};
	Buffer_Mul = { false, "", "", '.', -1, -1, -1};

	Cycle.push_back({ '+', 2 });
	Cycle.push_back({ '-', 2 });
	Cycle.push_back({ '*', 10 });
	Cycle.push_back({ '/', 40 });

	
}
void Run_Tomasulo() {    
	int cycle_number = 0; // ��l��0�A�ҥH�̫��X�ɭn�[1
	int issue_number = 0; // �ثeissue�����instruction�F // ���ǳ�issue�ĴX��instruction?
	int RS_pointer = -1;
	bool issue_add = false;

	while (true){
		// �Ĥ@��Cycle�ɡA�o��while�̥N��L���Ʀr�|�Ocycle_number = 0�C
		RS_pointer = -1;
		issue_add = false;

		// ------------------------------ Write Back ------------------------------
		// if buffer execution complete, write data back to RA and RF
		Tomasulo_Check_Buffer_Status(Buffer_Add, true, cycle_number);
		Tomasulo_Check_Buffer_Status(Buffer_Mul, false, cycle_number);

		// ------------------------------ Issue ------------------------------
		// check whether there have empty space in RS to issue data
		// int issue_pointer = -1; // 

		if (issue_number < Inst.size())
		{
			string type = Inst[issue_number].Type;
			if ((type == "ADD") || (type == "ADDI") || (type == "SUB") || (type == "SUBI"))
				for (int i = 0; i < 3; i++)
					if (RS[i].busy == false) { // �ѤW�ӤU��RS�Ŧ�
						RS[i].busy = true;
						RS_pointer = i;
						break;
					}

			if ((type == "MUL") || (type == "DIV"))
				for (int i = 3; i < 5; i++)
					if (RS[i].busy == false) {
						RS[i].busy = true;
						RS_pointer = i;
						break;
					}

			if (RS_pointer != -1) {
				Tomasulo_Issue(issue_number, RS_pointer); // RS_pointer�b�o�̬O�i�H��i�h����m
				Inst_history[issue_number].issue_cycle = (cycle_number + 1);
				issue_add = true;
			}
		}
		// ------------------------------ Dispatch ------------------------------
		if ((Buffer_Add.busy != true) || (Buffer_Mul.busy != true)) // buffer busy -> next cycle
			Tomasulo_Dispatch(true, cycle_number);                            

		if (issue_add == true)
			issue_number++;
		Cycle_print(cycle_number);
		cycle_number++;

		if (issue_number == Inst.size()) // Instruction��������->���X
		{
			if ((Buffer_Add.busy == false) && (Buffer_Mul.busy == false))
				break;
		}
	}

	
	//  
}
void Tomasulo_Check_Buffer_Status(RS_Buffer &buffer, bool ADD, int cycle_number)
{
	if ((buffer.busy == true) && (buffer.finish_cycle == (cycle_number + 1))){
		float result = 0;
		switch (buffer.operation){ // execute the result
		case '+':
			result = buffer.former + buffer.later;
			break;
		case '-':
			result = buffer.former - buffer.later;
			break;
		case '*':
			result = buffer.former * buffer.later;
			break;
		case '/':
			result = buffer.former / buffer.later;
			break;
		default:
			std::cout << "Something wrong!";
			break;
		}

		// �T�{RAT�����S���������W�r
		for (int i = 0; i < total_register_number; i++) { 
			if (RAT[i].replace_name == buffer.RS_name) { 
				RAT[i].replace_name = "";
				RF[i].value = result;
			}
		}

		// ��RS���۹�������Ƶ����N��
		for (int i = 0; i < 5; i++) {
			if (RS[i].former_s == buffer.RS_name) {
				RS[i].former = result;
				RS[i].former_s = "";
			}
			if (RS[i].later_s == buffer.RS_name) {
				RS[i].later = result;
				RS[i].later_s = "";
			}
		}
		//if (ADD == true){ 
		//	for (int i = 0; i < 3; i++) {
		//		if (RS[i].former_s == buffer.RS_name) {
		//			RS[i].former = result;
		//			RS[i].former_s = "";
		//		}
		//		if (RS[i].later_s == buffer.RS_name) {
		//			RS[i].later = result;
		//			RS[i].later_s = "";
		//		}
		//	}
		//}
		//if (ADD == false) {
		//	for (int i = 3; i < 5; i++) {
		//		if (RS[i].former_s == buffer.RS_name) {
		//			RS[i].former = result;
		//			RS[i].former_s = "";
		//		}
		//		if (RS[i].later_s == buffer.RS_name) {
		//			RS[i].later = result;
		//			RS[i].later_s = "";
		//		}
		//	}
		//}
		//  �M�ŭ쥻���ڦ�m��RS Line
		for (int i = 0; i < 5; i++) {
			if (RS[i].NAME == buffer.RS_name) {
				RS[i] = { buffer.RS_name, false, '.', NULL, NULL, "", "", false, false, 0 };
				break;
			}
		}

		// �M��Buffer�����
		buffer = { false, "","",'.', NULL, NULL, NULL};
		
	}
	else
		return;
}
void Tomasulo_Issue(int issue_number, int RS_pointer) {
	
	RS[RS_pointer].busy = true;
	RS[RS_pointer].execu_priority = issue_number;

		if ((Inst[issue_number].Type == "ADD") || (Inst[issue_number].Type == "ADDI")) 
			RS[RS_pointer].operation = '+';
		if ((Inst[issue_number].Type == "SUB") || (Inst[issue_number].Type == "SUBI")) 
			RS[RS_pointer].operation = '-';
		if (Inst[issue_number].Type == "MUL")
			RS[RS_pointer].operation = '*';
		if (Inst[issue_number].Type == "DIV")
			RS[RS_pointer].operation = '/';

		// Destinantion Register and InputA
		int RAT_pointer = 0;
		// InputA 
		// -> InputB + immediate
		// -> Destination // Destination�n�̫ᰵ�C�]������J�P��X��Register�W�r�@�˪����p
		Tomasulo_Issue_Check_RAT(1, Inst[issue_number].InputA_Reg.name, issue_number, RS_pointer);
		if ((Inst[issue_number].Type == "ADDI") || (Inst[issue_number].Type == "SUBI"))
			Tomasulo_Issue_Check_RAT(22, Inst[issue_number].InputB_Reg.name, issue_number, RS_pointer); // immediate
		else
			Tomasulo_Issue_Check_RAT(21, Inst[issue_number].InputB_Reg.name, issue_number, RS_pointer); // InputB
		Tomasulo_Issue_Check_RAT(0, Inst[issue_number].D_Reg.name, issue_number, RS_pointer);
}
void Tomasulo_Issue_Check_RAT(int D_Reg, string R, int issue_number, int RS_pointer){
		// D_Reg: 0_Destination, 1_InputA, 21_InputB(register), 22_InputB(immediate)

	if (D_Reg == 22) {
		RS[RS_pointer].later = Inst[issue_number].immed;
		return;
	}
	
	for (int i = 0; i < total_register_number; i++) {
		if (RAT[i].name == R) // �o�{RAT
		{
			if (D_Reg == 0) { // Destination Register -> ���ޭ쥻���S����Ƴ��n�мg�L�h			
				RAT[i].replace_name = RS[RS_pointer].NAME;
				break;
			}
			if (RAT[i].replace_name != "")
			{
				if (D_Reg == 1) // InputA
					RS[RS_pointer].former_s = RAT[i].replace_name;
				if (D_Reg == 21) // InputB
					RS[RS_pointer].later_s = RAT[i].replace_name;
				break;
			}
			else
			{
				if (D_Reg == 1) // InputA
					RS[RS_pointer].former = RF[i].value;
				if (D_Reg == 21) // InputB
					RS[RS_pointer].later = RF[i].value;
				break;
			}
		}
	}
}
void Tomasulo_Dispatch(bool d, int cycle)
{
	Tomasulo_Dispatch_Scan_RS(true, cycle); // �o��ӥi���i�H�X�@�_����?
	Tomasulo_Dispatch_Scan_RS(false, cycle);
	ready_set_sorting();// �o��n��@��ready_set_sorting
	int tag = 0;
	if (Buffer_Add.busy == false) {
		if (ready_set.size() > 0)
		{
			for (int i = 0; i < ready_set.size(); i++) {
				if (ready_set[i] <= 2){ // 0, 1, 2
					tag = i;
					RS[ready_set[tag]].execution = true;
					Inst_history[RS[ready_set[tag]].execu_priority].execu_cycle = cycle;
					Tomasulo_Dispatch_Buffer_Assign(Buffer_Add, ready_set[tag], cycle);
					ready_set.erase(ready_set.begin() + tag);// �ݭn��w�g���n�����ȱqready_set���R��
					break;
				}
			}
			
		}
	}
	if (Buffer_Mul.busy == false) {
		if (ready_set.size() > 0)
		{
			int tag = 0;
			for (int i = 0; i < ready_set.size(); i++) {
				if (ready_set[i] > 2) { // 3, 4
					tag = i;
					RS[ready_set[tag]].execution = true;
					Inst_history[RS[ready_set[tag]].execu_priority].execu_cycle = (cycle + 1);
					Tomasulo_Dispatch_Buffer_Assign(Buffer_Mul, ready_set[tag], cycle);
					ready_set.erase(ready_set.begin() + tag);
					break;
				}
			}

		}
	}
	// ready_set have number
		// Add
		// Mul
	// ready_set empty
		// Add 
		// Mul 
}
void Tomasulo_Dispatch_Scan_RS(bool Add, int cycle) {
	if (Add) {
		for (int i = 0; i < 3; i++) {
			if (((((((RS[i].former != NULL) && (RS[i].later != NULL)) && 
				(RS[i].former_s == "")) && (RS[i].later_s == "")) &&
				(cycle != RS[i].execu_priority)) && (RS[i].execution == false)) && (RS[i].dispatch == false))
			{
				RS[i].dispatch = true;
				ready_set.push_back(i);
			}
		}
	}
	else {
		for (int i = 3; i < 5; i++) {
			if (((((((RS[i].former != NULL) && (RS[i].later != NULL)) &&
				(RS[i].former_s == "")) && (RS[i].later_s == "")) &&
				(cycle != RS[i].execu_priority)) && (RS[i].execution == false)) && (RS[i].dispatch == false))
			{
				RS[i].dispatch = true;
				ready_set.push_back(i);
			}
		}
	}
}
void Tomasulo_Dispatch_Buffer_Assign(RS_Buffer &buffer, int RS_pointer, int cycle) {
	// �n����|��execution_cycle ��wb_cycle���Ʀr�g�iInst_history
	buffer.busy = true;
	buffer.RS_name = RS[RS_pointer].NAME;
	buffer.operation = RS[RS_pointer].operation;
	buffer.former = RS[RS_pointer].former;
	buffer.later = RS[RS_pointer].later;
	switch (buffer.operation) {
	case '+':
	case '-':
		buffer.finish_cycle = (cycle+1) + 2;
		break;
	case '*':
		buffer.finish_cycle = (cycle + 1) + 10;
		break;
	case '/':
		buffer.finish_cycle = (cycle + 1) + 40;
		break;
	}
}

void ready_set_sorting() {
	if (ready_set.size() > 0)
	{
		int temp = 0;
		for (int i = 0; i < ready_set.size(); i++) {
			for (int j = 0; j < ready_set.size(); j++) {
				if (RS[ready_set[i]].execu_priority < RS[ready_set[j]].execu_priority) {
					temp = ready_set[j];
					ready_set[j] = ready_set[i];
					ready_set[i] = temp;
				}
			}
		}
	}
}
void Cycle_print(int cycle_number)
{
	cout << "Cycle: " << (cycle_number + 1) << endl;
	// cycle_number = cycle_number - 1;

	// ��XRF
	cout << "�iRF�j" << endl;
	for (int i = 0; i < total_register_number; i++) 
		cout << "[" << RF[i].name << ":" << RF[i].value << "] ";

	// ��XRAT
	cout << endl << "�iRAT�j" << endl;
	for(int i = 0; i <total_register_number; i++)
		cout << "[" << RAT[i].name << ":" << RAT[i].replace_name << "] ";

	// ��XALU
	string f, l;
	cout << endl << "�iRS�j" << endl;
	for (int i = 0; i < RS.size(); i++) {
		cout << RS[i].NAME;
			if (RS[i].busy == true){
				f = (RS[i].former_s == "") ? to_string(RS[i].former) : RS[i].former_s;
				l = (RS[i].later_s == "") ? to_string(RS[i].later) : RS[i].later_s;
				cout << " " << RS[i].operation << " " << f << " " << l;
			}
			cout << endl;
	}
	cout << "Buffer_Add: (" << Buffer_Add.RS_name << ") ";
	if (Buffer_Add.busy == true)
		cout << Buffer_Add.former << " " << Buffer_Add.operation << " " << Buffer_Add.later;
	cout << endl<< "Buffer_Mul: (" << Buffer_Mul.RS_name << ") ";
	if(Buffer_Mul.busy == true)
		cout<< Buffer_Mul.former << " " << Buffer_Mul.operation << " " << Buffer_Mul.later << endl;

	cout << endl << endl;
}
// �T�w�i�H���`Ū���奻
	// �@�~2�S��Address���B�z�A�ݭn��Code
// �����Print Out���覡�T�{�ݭn���Ǹ�Ƶ��c & ��l�ƥL��
// �T�wParser�i�H���`�B�@(�p�g��j�g�B��Jump���Ǫ��������R��)


// Buffer���???QwQ
