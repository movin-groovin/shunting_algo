
// g++ -std=c++11 chunting.cpp -o chunt

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

#include <cmath>



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
			return tokens::is_digit(t.val);
		}
		virtual bool IsFunction(const tokens::TOKEN & t) const  {
			return t.type == tokens::function;
		}
		virtual bool IsSeparator(const tokens::TOKEN & t) const  {
			return t.val == std::string(1, ',');
		}
		virtual bool IsOperator(const tokens::TOKEN & t) const  {
			std::string ops ("+-*/");
			return ops.find(t.val[0]) != ops.npos;
		}
		virtual bool IsOpenBracket(const tokens::TOKEN & t) const  {
			return t.val == std::string(1, '(');
		}
		virtual bool IsCloseBracket(const tokens::TOKEN & t) const  {
			return t.val == std::string(1, ')');
		}
		virtual int GetPriority(const tokens::TOKEN & t) const { // now we think, that all our operators are left associative
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
	T CalculateRPN (std::deque<tokens::TOKEN> && q) {
		std::deque<tokens::TOKEN> toks(q);
		
		;
		
		return T();
	}
	
}


int main () {
	std::string cmds = "(1 + pow(3, 4) + (3 -1)*4) / 3";
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
