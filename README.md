# Tomasulo-s-_Law
2019 Data Structure Course Homework Two
# Cache_Replacement
2019 Data Structure Course Homework Three

### 題目: Tomasulo's Law

### 使用語言: C++

### 要求:
    1.讀取一個檔案
    2.可以自行設定Block size
    3.可以自行設定Cache total size    
    4.可以自行設定Set associative number
    5.提供至少兩種cache replacement strategy，並可以選擇要使用哪一種strategy

### 編譯方式:
	1.使用Dev-C++、Visual Studio等程式完成編譯
	2.linux "g++ hw2.cpp ASSEMBLY_HW2.txt -o hw3"
	
### 範例input(using txt file "HW3.txt"):
	ADDI F1, F2, 1
	SUB F1, F3, F4
	DIV F1, F2, F3
	MUL F2, F3, F4
	ADD F2, F4, F2
	ADDI F4, F1, 2
	MUL F5, F5, F5
	ADD F1, F4, F4	

### 範例output:

	output會輸出所有cycle。
	內容包括RF、RAT、RS與加法乘法ALU當前正在運算的內。
![image](https://github.com/sam34andy/Tomasulo-s-_Law/blob/master/output1.JPG)
![image](https://github.com/sam34andy/Tomasulo-s-_Law/blob/master/output2.JPG)

### 程式碼內容:
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
	Run_Tomasulo 
		-> Tomasulo_Check_Buffer_Status //確認當前兩個ALU是否有instruction執行完畢。釋出空間，讓相對應準備好的其他instruction開始執行。
		-> Tomasulo_Issue //執行Issue
			-> Tomasulo_Issue_Check_RAT //確認RF與RAT中的名字，將正確的Register name放進RS中
		-> Tomasulo_Dispatch //執行Dispatch
			-> Tomasulo_Dispatch_Scan_RS //確認當前RS中有沒有已經可以Dispatch的instruction
			-> Tomasulo_Dispatch_Buffer_Assign //把Dispatch的instruction寫進相對應的加/乘法ALU中
			-> ready_set_sorting //ready_set確保先issue的instruction會先被執行。
						
