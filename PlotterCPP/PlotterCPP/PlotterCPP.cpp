#include "pch.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <Windows.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <iterator>
#include <gl/GLUT.h>
using namespace std;
int countd = 0;  // Размер масива У,Х
double* X; // масив для хранение х-ов
double* Y; // масив для хранения у-ов
double range;
void display() {                    // Функция перерисовки дисплея
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_LINES);

	glColor3f(0, 1, 0);
	glVertex2f(0, 50);   // Рисование системы координат
	glVertex2f(0, -50);
	glVertex2f(-50, 0);
	glVertex2f(50, 0);

	for (int i = -11; i < 8; i++) {
		glVertex2f(10 + i * 5, -1);
		glVertex2f(10 + i * 5, 1);
	}

	for (int i = -11; i < 8; i++) {
		glVertex2f(-1, 10 + i * 5);
		glVertex2f(1, 10 + i * 5);
	}// Конец рисования системы кooрдинат

	glEnd();

	glBegin(GL_POINTS); // Рисование точок графика функции
	for (int i = 0; i < countd; i++) {
		glVertex2f(X[i], Y[i]);
	}
	glEnd();
	glFinish();
	//glutSwapBuffers();
}
void mydisplay() {
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_LINES);

	glColor3f(0, 1, 0);
	glVertex2f(0, range);
	glVertex2f(0, -range);
	glVertex2f(-range, 0);
	glVertex2f(range, 0);

	for (int i = -range; i <= range; i++) {
		glVertex2f(-0.5, i);
		glVertex2f(0.5, i);
		glVertex2f(i, -0.5);
		glVertex2f(i, 0.5);
	}
	glEnd();
	glBegin(GL_POINTS);
	glColor3f(1, 0, 0);
	for (int k = 0; k < countd; k++)
		glVertex2f(X[k], Y[k]);
	glEnd();
	glFlush();
}
void Start_glut(int *argc, char** argv) {
	// Стандартное создание окна в OpenGl
	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(700, 700);
	glutInitWindowPosition(200, 0);
	glutCreateWindow("Graphics");
	glClearColor(0, 0, 0, 1);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-range, range, -range, range, -range, range);
	glutDisplayFunc(mydisplay);
	glutMainLoop();
	delete[] X;
	delete[] Y;
}
bool check_iter_num(string::iterator iter) {
	if (*iter == '0' || *iter == '1' || *iter == '2' || *iter == '3' || *iter == '4' || *iter == '5' ||
		*iter == '6' || *iter == '7' || *iter == '8' || *iter == '9' || *iter == '.')
		return true;
	return false;
}
double trigonometry(string& func, double num) {
	if (func == "sin")
		return sin(num);
	else if (func == "cos")
		return cos(num);
	else if (func == "tan")
		return tan(num);
	else if (func == "ctg")
		return 1 / tan(num);
	else if (func == "asin")
		return asin(num);
	else if (func == "acos")
		return acos(num);
	else if (func == "atan")
		return atan(num);
	else return 0;
}
void x_substitute(const double x, string& equation) {
	for (string::iterator iter = equation.begin(); iter != equation.end(); iter++) {
		if (*iter == 'x') {
			iter = equation.erase(iter);
			string ar;
			if (x < 0)
				ar = "(" + to_string(x) + ")";
			else
				ar = to_string(x);
			iter = equation.insert(iter, ar.begin(), ar.end());
			iter += to_string(x).size() - 2;
			//cout << x << ", " << to_string(x).size() << endl;
		}
	}
}
double operate(double left, char op, double right) {
	if (op == '+')
		return left + right;
	else if (op == '-')
		return left - right;
	else if (op == '*')
		return left * right;
	else if (op == '/')
		return left / right;
	else if (op == '^')
		return pow(left, right);
}
bool check_continue_string(string::iterator iter, int br) {
	if (*iter != '-' && *iter != '+') {
		return true;
	}
	else if (br == 0)
		return false;
	else return true;
}
string next_operand_string(string equation) {
	string result;
	int br = 0;
	for (string::iterator iter = equation.begin(); iter != equation.end() && check_continue_string(iter, br); ++iter) {
		if (*iter == '(')
			br++;
		else if (*iter == ')')
			br--;
		result += *iter;
	}
	//cout << "next operand string = " << result << endl;
	return result;
}
double calculate(string equation) {
	double num = 0;
	for (string::iterator iter = equation.begin(); iter != equation.end(); ++iter) {
		if (check_iter_num(iter)) {
			string number;
			if (*iter == '-')
				number = *iter++;
			for (iter; iter != equation.end() && check_iter_num(iter); iter++) {
				number += *iter;
			}
			--iter;
			istringstream iss(number);
			iss >> num;
			//result = num;
		}
		else if (*iter == '+' || *iter == '-' || *iter == '*' || *iter == '/' || *iter == '^') {
			string next_str = next_operand_string(string(++iter, equation.end()));
			num /*= result*/ = operate(num, *(--iter), calculate(next_str));
			iter += next_str.size();
		}
		else if (*iter == 's' || *iter == 'c' || *iter == 't' || *iter == 'a') {
			string trigon_func, argument;
			while (*iter != '(')
				trigon_func += *iter++;
			iter++;
			int br = 1;
			for (iter; br != 0; iter++) {
				if (*iter == '(') {
					br++;
					argument += *iter;
				}
				else if (*iter == ')') {
					br--;
					argument += *iter;
					if (br == 0)
						argument.pop_back();
				}
				else argument += *iter;
			}
			num /*= result*/ = trigonometry(trigon_func, calculate(argument));
			iter--;
		}
		else if (*iter == '(') {
			iter++;
			int br_count = 1;
			string new_equation;
			for (iter; br_count != 0; iter++) {
				if (*iter == '(') {
					br_count++;
					new_equation += *iter;
				}
				else if (*iter == ')') {
					br_count--;
					new_equation += *iter;
					if (br_count == 0)
						new_equation.pop_back();
				}
				else if (iter == equation.end()) cout << "Скобки не закрылись до конца выражения!" << endl;
				else new_equation += *iter;
			}
			//cout << "Длинна выражения в скобках = " << new_equation.size() << endl;
			num /*= result*/ = calculate(new_equation);
			iter--;
		}
	}
	return num;
}
double count_Y(double x, string equation) {
	//cout << endl << equation << endl;
	x_substitute(x, equation);
	//cout << endl << equation << endl;
	return calculate(equation);
}
double Step(int a) {
	switch (a) {
	case 2:
		return 0.05;
	case 3:
		return 0.02;
	case 4:
		return 0.01;
	default:
		return 0.1;
	}
}
bool buildarray(const string& s) {
	double step, beg, end;
	cout << "Функция: " << s << endl;
	//cin >> beg >> end;
	beg = -range; end = range;
	cout << "Шаг по х: (1. - 0.1, 2. - 0.05, 3. - 0.02, 4. - 0.01): ";
	cin >> step;
	step = Step(step);
	int size = (int)((end - beg) / step);
	countd = size;
	X = new double[size];
	Y = new double[size];
	cout << "Точек: " << size + 1 << endl;
	for (int i = 0; i <= size; i++) {
		X[i] = beg + i * step;
	}
	for (int i = 0; i <= size; i++) {
		Y[i] = count_Y(X[i], s);
	}
	cout << "Массивы координат сгенерированы :" << endl;
	//for (int i = 0; i <= size; i++)
		//cout << "(" << X[i] << ", " << Y[i] << ")" << endl;
	return true;
}
void funclist(istream& is, vector<string>& vec) {
	int k = 0;
	string x;
	while (getline(is, x)) {
		cout << ++k << ". " << x << endl;
		vec.push_back(x);
	}
}
int main(int argc, char** argv)
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	//vector<float> arr;  // Вектор в котором будем хранить даные из файла
	vector<string> func;
	ifstream fin;       // Для вывода данных из файла
	ofstream fout;
	string path = "C:\\file2.txt", x;
	int commands = 0, command, fnum;
	while (commands <= 20) {
		while (!fin.is_open()) {                      // Цикл открытия файла
			fin.open(path);
			fout.open(path, ios::app);
			if (!fin.is_open()) {
				std::cout << "Error: File not find!\n";
			}
		}
		cout << "1 - Построить последнюю функцию из файла\n2 - построить функцию из файла\n3 - ввести и построить новую функцию\n4 - Удалить функцию: ";
		cin >> command;
		switch (command) {
		case 1:
			while (getline(fin, x));  // Запись из файла в вектор
			fin.close();
			if (!buildarray(x))
				cout << "Функция не может быть построена" << endl;
			else
				Start_glut(&argc, argv);
			break;
		case 2:
			funclist(fin, func);
			fin.close();
			cout << "Введите номер функции: ";
			do {
				cin >> fnum;
				if (fnum > func.size() || fnum < 0)  cout << "Номера не существует, введите заного: ";
				if (fnum == 0) break;
			} while (fnum > func.size());
			cout << "Построение функции " << func[fnum - 1] << endl << endl;
			if (!buildarray(func[fnum - 1]))
				cout << "Функция не может быть построена" << endl;
			else
				Start_glut(&argc, argv);
			func.clear();
			break;
		case 3:
			cin.ignore();
			cout << "Функция: ";
			getline(cin, x);
			fout << endl << x;
			fout.close();
			if (!buildarray(x))
				cout << "Функция не может быть построена" << endl;
			else
				Start_glut(&argc, argv);
			break;
		case 4:
			funclist(fin, func);
			fin.close();
			cout << "Номер удаляемой функции: ";
			do {
				cin >> fnum;
				if (fnum > func.size())  cout << "Номера не существует, введите заного: ";
			} while (fnum > func.size());
			func.erase(func.begin() + fnum - 1);
			fout.close();
			fout.open(path);
			copy(func.begin(), func.end(), ostream_iterator<string>(fout, "\n"));
			func.clear();
			fout.close();
			break;
		}
		commands++;
	}
	return 0;
}