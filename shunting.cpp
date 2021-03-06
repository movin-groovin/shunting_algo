
// g++ -std=c++11 shunting.cpp -o shunt

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <queue>
#include <deque>
#include <stack>
#include <sstream>
#include <stdexcept>
#include <iterator>

#include <cmath>
#include <cassert>



namespace tokens {

	const std::vector<std::string> var_func = {"pow", "abs", "cos", "sin"};
	const std::vector<std::string> var_delims = {"+", "-", "*", "/", "(", ")", ",", " "};
	

	enum token_type {
		function = 0,
		delimeter,
		operand // now is only number
	};
	
	bool is_digit(const std::string & s) {
		char min = '0', max = '9';
		
		for (auto ch : s) {
			if (!(ch >= min && ch <= max)) {
				return false;
			}
			//
		}
		return true;
	}
	
	bool is_space(char ch) {
		return ch == ' ';
	}

	typedef struct _TOKEN {
		token_type type;
		std::string val;
		
		_TOKEN (token_type t, const std::string & s): type(t), val(s) {}
		_TOKEN (const _TOKEN & t_ref): type(t_ref.type), val(t_ref.val) {}
	} TOKEN, *PTOKEN;


	class CTokenizer {
	public:
		CTokenizer(
			const std::vector<std::string> & f_names,
			const std::vector<std::string> & delims,
			std::istream & is
		): m_func_names(f_names), m_delimeters(delims)
		{
			int ch;
			std::string token;
			
			while ((ch = is.get()) != EOF) {
				if (std::find(m_delimeters.begin(), m_delimeters.end(), std::string(1, static_cast<char>(ch))) != m_delimeters.end()) {
					if (!token.empty()) {
						if (is_digit(token))
							m_tokens.push(TOKEN(operand, token));
						else
							m_tokens.push(TOKEN(function, token));
						token.clear();
					}
					if (!is_space(static_cast<char>(ch)))
						m_tokens.push(TOKEN(delimeter, std::string(1, static_cast<char>(ch))));
					continue;
				}
				else {
					token += static_cast<char>(ch);
				}
				// 
			}
			
			if (!token.empty()) {
				if (is_digit(token))
					m_tokens.push(TOKEN(operand, token));
				else
					m_tokens.push(TOKEN(function, token));
			}
			
			return;
		}
		virtual ~CTokenizer() {}
		
		std::queue<TOKEN> GetTokens() const {
			return m_tokens;
		}
		
	private:
		std::vector<std::string> m_func_names;
		std::vector<std::string> m_delimeters;
		std::queue<TOKEN> m_tokens;
	};
	
}


namespace schunting {
	
	class CSchuntingAlgo {
	private:
		void ConstructRPN();
		
	public:
		CSchuntingAlgo(const std::queue<tokens::TOKEN> & t_q): m_tokens(t_q) {
			ConstructRPN();
		}
		virtual ~CSchuntingAlgo () {}
		
		virtual bool IsInteger(const tokens::TOKEN & t) const  {
			assert(!t.val.empty());
			return tokens::is_digit(t.val);
		}
		virtual bool IsFunction(const tokens::TOKEN & t) const  {
			assert(!t.val.empty());
			return t.type == tokens::function;
		}
		virtual bool IsSeparator(const tokens::TOKEN & t) const  {
			assert(!t.val.empty());
			return t.val == std::string(1, ',');
		}
		virtual bool IsOperator(const tokens::TOKEN & t) const  {
			assert(!t.val.empty());
			std::string ops ("+-*/");
			return ops.find(t.val[0]) != ops.npos;
		}
		virtual bool IsOpenBracket(const tokens::TOKEN & t) const  {
			assert(!t.val.empty());
			return t.val == std::string(1, '(');
		}
		virtual bool IsCloseBracket(const tokens::TOKEN & t) const  {
			assert(!t.val.empty());
			return t.val == std::string(1, ')');
		}
		virtual int GetPriority(const tokens::TOKEN & t) const { // now we think, that all our operators are left associative
			assert(!t.val.empty());
			return (t.val == "+" || t.val == "-") ? 1 : 2; // "+-" - 1; "*/" - 2
		}
		
		std::deque<tokens::TOKEN> GetResults() const {
			return m_queue;
		}
		
	private:
		std::queue<tokens::TOKEN> m_tokens;
		
		std::stack<tokens::TOKEN> m_stack;
		std::deque<tokens::TOKEN> m_queue;
	};
	
	void CSchuntingAlgo::ConstructRPN() {
		// int var = 0;
		
		while(!m_tokens.empty()) {
			tokens::TOKEN tok(m_tokens.front());
			m_tokens.pop();
			
			if (IsInteger(tok)) {
				m_queue.push_back(tok);
			}
			else if (IsFunction(tok)) {
				m_stack.push(tok);
			}
			else if (IsSeparator(tok)) {
				while (!IsOpenBracket(m_stack.top())) {
					m_queue.push_back(m_stack.top());
					m_stack.pop();
				}
				
				if (m_stack.empty())
					throw std::runtime_error("Parse error: is absent func parameter separator (',') or open bracket ('(')");
			}
			else if (IsOperator(tok)) {
				while (
					!m_stack.empty() &&
					IsOperator(m_stack.top()) &&
					(GetPriority(tok) <= GetPriority(m_stack.top()))
				) {
					m_queue.push_back(m_stack.top());
					m_stack.pop();
				}
				
				m_stack.push(tok);
			}
			else if (IsOpenBracket(tok)) {
				m_stack.push(tok);
			}
			else if (IsCloseBracket(tok)) {
				while(!m_stack.empty() && !IsOpenBracket(m_stack.top())) {
					m_queue.push_back(m_stack.top());
					m_stack.pop();
				}
				
				if (m_stack.empty())
					throw std::runtime_error("Parse error: is absent opening bracket ('(')");
					
				m_stack.pop();
				if (!m_stack.empty() && IsFunction(m_stack.top())) {
					m_queue.push_back(m_stack.top());
					m_stack.pop();
				}
			}
			//
		}
		
		while(!m_stack.empty()) {
			if (IsOpenBracket(m_stack.top()))
				throw std::runtime_error("Parse error: is absent closing bracket (')')");
			m_queue.push_back(m_stack.top());
			m_stack.pop();
		}
		
		return;
	}
	
	template <typename T>
	struct NEW_TOKEN {
		tokens::token_type type;
		std::string val;
		T dig_val;
		
		NEW_TOKEN (tokens::token_type t, const std::string & s, T d_val): type(t), val(s), dig_val(d_val) {}
		NEW_TOKEN (const NEW_TOKEN & t_ref): type(t_ref.type), val(t_ref.val), dig_val(t_ref.dig_val) {}
		
		void swap(NEW_TOKEN & t) {
			std::swap(type, t.type);
			std::swap(val, t.val);
			std::swap(dig_val, t.dig_val);
		}
	};
	
	template <typename T>
	T CalculateRPN (std::deque<tokens::TOKEN> && q) {
		std::deque<tokens::TOKEN> toks(q);
		std::stack<NEW_TOKEN<T> > s_calcs;
		
		while (!toks.empty()) {
			tokens::TOKEN t(toks.front());
			toks.pop_front();
			
			assert(t.type == tokens::operand || t.type == tokens::delimeter || t.type == tokens::function);
			
			if (t.type == tokens::operand) {
				T tmp;
				std::istringstream iss(t.val);
				iss >> tmp;
				s_calcs.push(NEW_TOKEN<T>(t.type, t.val, tmp));
			}
			else if (t.type == tokens::delimeter) {
				if (s_calcs.empty())
					throw std::runtime_error("Incorrect RPN note");
				NEW_TOKEN<T> t1(s_calcs.top());
				s_calcs.pop();
				
				if (s_calcs.empty())
					throw std::runtime_error("Incorrect RPN note");
				NEW_TOKEN<T> t2(s_calcs.top());
				s_calcs.pop();
				
				t1.swap(t2); // change order at moving from queue to stack and pop back
				
				assert(t1.type == tokens::operand && t2.type == tokens::operand);
				
				if (t.val == "+") {
					s_calcs.push(NEW_TOKEN<T>(t1.type, "", t1.dig_val + t2.dig_val));
				}
				else if (t.val == "-") {
					s_calcs.push(NEW_TOKEN<T>(t1.type, "", t1.dig_val - t2.dig_val));
				}
				else if (t.val == "*") {
					s_calcs.push(NEW_TOKEN<T>(t1.type, "", t1.dig_val * t2.dig_val));
				}
				else if (t.val == "/") {
					s_calcs.push(NEW_TOKEN<T>(t1.type, "", t1.dig_val / t2.dig_val));
				}
				//
			}
			else { // function
				if (t.val == "pow") {
					if (s_calcs.empty())
						throw std::runtime_error("Incorrect RPN note");
					NEW_TOKEN<T> t1(s_calcs.top());
					s_calcs.pop();
					
					if (s_calcs.empty())
						throw std::runtime_error("Incorrect RPN note");
					NEW_TOKEN<T> t2(s_calcs.top());
					s_calcs.pop();
					
					t1.swap(t2); // change order at moving from queue to stack and pop back
					
					assert(t1.type == tokens::operand && t2.type == tokens::operand);
					
					s_calcs.push( NEW_TOKEN<T>( t1.type, "", std::pow(t1.dig_val, t2.dig_val) ) );
				}
				else {
					if (s_calcs.empty())
						throw std::runtime_error("Incorrect RPN note");
					NEW_TOKEN<T> t1(s_calcs.top());
					s_calcs.pop();
					
					assert(t1.type == tokens::operand);
					
					if (t.val == "abs")
						s_calcs.push( NEW_TOKEN<T>( t1.type, "", std::abs(t1.dig_val) ) );
					else if (t.val == "sin")
						s_calcs.push( NEW_TOKEN<T>( t1.type, "", std::sin(t1.dig_val) ) );
					else if (t.val == "cos")
						s_calcs.push( NEW_TOKEN<T>( t1.type, "", std::cos(t1.dig_val) ) );
					//
				}
				//
			}
			//
		}
		
		return s_calcs.top().dig_val;
	}
	
}


std::string GetData(std::istream & is) {
	std::string buf;
	//std::copy(std::istream_iterator<char>(is), std::istream_iterator<char>(), std::back_insert_iterator<std::string>(buf));
	std::getline(is, buf, '\n');
	return buf;
}


int main (int argc, char **argv) {
	std::string cmds = "(1 + pow(3, 4) + (3 -1)*4) / 3";
	
	std::cout << "Enter formula: ";
	cmds = GetData(std::cin);
#if !defined(NDEBUG)
	std::cout << "Formula: " << cmds << "\n";
#endif
	
	std::istringstream iss(cmds);
	tokens::CTokenizer toks (tokens::var_func, tokens::var_delims, iss);
	
	/*
	std::queue<tokens::TOKEN> dat_toks = toks.GetTokens();
	while(!dat_toks.empty()) {
		tokens::TOKEN t(dat_toks.front());
		dat_toks.pop();
		
		std::cout << t.type << "; " << t.val << "\n";
	}
	*/
	try {
		schunting::CSchuntingAlgo chnt(toks.GetTokens());
		std::cout << "Results: " << schunting::CalculateRPN<long long> (chnt.GetResults()) << std::endl;
	} catch (std::exception & Exc) {
		std::cout << "Have caught an exception: " << Exc.what() << "\n";
		return 1001;
	}
	
	
	return 0;
}
