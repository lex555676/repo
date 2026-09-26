#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

struct Transformer;
struct Number;
struct BinaryOperation;
struct FunctionCall;
struct Variable;

struct Expression
{
	virtual ~Expression() {}
	virtual double evaluate() const = 0;
	virtual Expression* transform(Transformer* tr) const = 0;
};

struct Transformer // pattern Visitor
{
	virtual ~Transformer() {}
	virtual Expression* transformNumber(Number const*) = 0;
	virtual Expression* transformBinaryOperation(BinaryOperation const*) = 0;
	virtual Expression* transformFunctionCall(FunctionCall const*) = 0;
	virtual Expression* transformVariable(Variable const*) = 0;
};

struct Number : Expression
{
	Number(double value) : value_(value) {}
	double value() const { return value_; }
	double evaluate() const override { return value_; }
	Expression* transform(Transformer* tr) const override {
		return tr->transformNumber(this);
	}
private:
	double value_;
};

struct BinaryOperation : Expression
{
	enum {
		PLUS = '+',
		MINUS = '-',
		DIV = '/',
		MUL = '*'
	};
	BinaryOperation(Expression const* left, int op, Expression const* right)
		: left_(left), op_(op), right_(right) {}
	~BinaryOperation() { delete left_; delete right_; }
	double evaluate() const override {
		double l = left_->evaluate();
		double r = right_->evaluate();
		switch (op_) {
		case BinaryOperation::PLUS: return l + r;
		case BinaryOperation::MINUS: return l - r;
		case BinaryOperation::DIV: return l / r;
		case BinaryOperation::MUL: return l * r;
		}
		return 0.0;
	}
	Expression* transform(Transformer* tr) const override {
		return tr->transformBinaryOperation(this);
	}
	Expression const* left() const { return left_; }
	Expression const* right() const { return right_; }
	int operation() const { return op_; }
private:
	Expression const* left_;
	Expression const* right_;
	int op_;
};

struct FunctionCall : Expression
{
	FunctionCall(std::string const& name, Expression const* arg)
		: name_(name), arg_(arg) {}
	~FunctionCall() { delete arg_; }
	double evaluate() const override {
		double a = arg_->evaluate();
		if (name_ == "sqrt") return sqrt(a);
		if (name_ == "abs") return fabs(a);
		return 0.0;
	}
	Expression* transform(Transformer* tr) const override {
		return tr->transformFunctionCall(this);
	}
	std::string const& name() const { return name_; }
	Expression const* arg() const { return arg_; }
private:
	std::string const name_;
	Expression const* arg_;
};

struct Variable : Expression
{
	Variable(std::string const name) : name_(name) {}
	std::string const& name() const { return name_; }
	double evaluate() const override { return 0.0; }
	Expression* transform(Transformer* tr) const override {
		return tr->transformVariable(this);
	}
private:
	std::string const name_;
};

//ЗАДАНИЕ 1
struct CopySyntaxTree : Transformer
{
	Expression* transformNumber(Number const* number) override {
		return new Number(number->value());
	}
	Expression* transformBinaryOperation(BinaryOperation const* binop) override {
		Expression* new_left = binop->left()->transform(this);
		Expression* new_right = binop->right()->transform(this);
		return new BinaryOperation(new_left, binop->operation(), new_right);
	}
	Expression* transformFunctionCall(FunctionCall const* fcall) override {
		Expression* new_arg = fcall->arg()->transform(this);
		return new FunctionCall(fcall->name(), new_arg);
	}
	Expression* transformVariable(Variable const* var) override {
		return new Variable(var->name());
	}
};

// ЗАДАНИЕ 2
struct FoldConstants : Transformer
{
	Expression* transformNumber(Number const* number) override {
		return new Number(number->value());
	}
	Expression* transformBinaryOperation(BinaryOperation const* binop) override {
		Expression* left = binop->left()->transform(this);
		Expression* right = binop->right()->transform(this);

		Number* numLeft = dynamic_cast<Number*>(left);
		Number* numRight = dynamic_cast<Number*>(right);

		if (numLeft && numRight) {
			double result = 0.0;
			double l = numLeft->value();
			double r = numRight->value();
			switch (binop->operation()) {
			case BinaryOperation::PLUS:  result = l + r; break;
			case BinaryOperation::MINUS: result = l - r; break;
			case BinaryOperation::DIV:   result = l / r; break;
			case BinaryOperation::MUL:   result = l * r; break;
			}
			delete left;
			delete right;
			return new Number(result);
		}
		return new BinaryOperation(left, binop->operation(), right);
	}
	Expression* transformFunctionCall(FunctionCall const* fcall) override {
		Expression* arg = fcall->arg()->transform(this);
		Number* numArg = dynamic_cast<Number*>(arg);

		if (numArg) {
			double result = 0.0;
			double a = numArg->value();
			if (fcall->name() == "sqrt") result = sqrt(a);
			else if (fcall->name() == "abs") result = fabs(a);
			delete arg;
			return new Number(result);
		}
		return new FunctionCall(fcall->name(), arg);
	}
	Expression* transformVariable(Variable const* var) override {
		return new Variable(var->name());
	}
};

// для вывода дерева
struct PrintVisitor : Transformer
{
	Expression* transformNumber(Number const* number) override {
		std::cout << number->value();
		return nullptr;
	}
	Expression* transformBinaryOperation(BinaryOperation const* binop) override {
		std::cout << "(";
		binop->left()->transform(this);
		std::cout << " " << (char)binop->operation() << " ";
		binop->right()->transform(this);
		std::cout << ")";
		return nullptr;
	}
	Expression* transformFunctionCall(FunctionCall const* fcall) override {
		std::cout << fcall->name() << "(";
		fcall->arg()->transform(this);
		std::cout << ")";
		return nullptr;
	}
	Expression* transformVariable(Variable const* var) override {
		std::cout << var->name();
		return nullptr;
	}
};

int main()
{
	setlocale(LC_ALL, "Russian");

	Number* n32 = new Number(32.0);
	Number* n16 = new Number(16.0);
	BinaryOperation* minus = new BinaryOperation(n32, BinaryOperation::MINUS, n16);
	FunctionCall* callSqrt = new FunctionCall("sqrt", minus);
	Variable* var = new Variable("var");
	BinaryOperation* mult = new BinaryOperation(var, BinaryOperation::MUL, callSqrt);
	FunctionCall* callAbs = new FunctionCall("abs", mult);

	PrintVisitor printer;
	std::cout << "ИСХОДНОЕ ДЕРЕВО:";
	callAbs->transform(&printer);
	std::cout << std::endl;

	//Проверка 1 к опирование
	CopySyntaxTree CST;
	Expression* copiedExpr = callAbs->transform(&CST);
	std::cout << "Копия дерева: ";
	copiedExpr->transform(&printer);
	std::cout << "\nАдрес оригинала: " << callAbs << " | Адрес копии: " << copiedExpr << std::endl;

	std::cout << std::endl;

	//Проверка 2 свёртки констант
	FoldConstants FC;
	Expression* foldedExpr = callAbs->transform(&FC);
	std::cout << "После свертки констант: ";
	foldedExpr->transform(&printer);
	std::cout << "\nРезультат: " << foldedExpr->evaluate() << std::endl;

	delete callAbs;
	delete copiedExpr;
	delete foldedExpr;

	return 0;
}