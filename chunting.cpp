
// g++ -std=c++11 chunting.cpp -o chunt

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <queue>
#include <sstream>

#include <cmath>



namespace tokens {

	const std::vector<std::string> var_func = {"pow", "abs", "cos", "sin"};
	const std::vector<std::string> var_delims = {"+", "-", "*", "/", "(", ")", ",", " "};
	

	enum token_type {
		function = 0,
		delimeter,
		operand
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


int main () {
	std::string cmds = "(1 + pow(3, 4) + (3 -1)*4) / 3";
	std::istringstream iss(cmds);
	tokens::CTokenizer toks (tokens::var_func, tokens::var_delims, iss);
	std::queue<tokens::TOKEN> dat_toks = toks.GetTokens();
	
	
	while(!dat_toks.empty()) {
		tokens::TOKEN t(dat_toks.front());
		dat_toks.pop();
		
		std::cout << t.type << "; " << t.val << "\n";
	}
	
	
	return 0;
}
