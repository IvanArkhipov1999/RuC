/*
 *	Copyright 2021 Andrey Terekhov, Ilya Andreev
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 */

#include "lexer.h"
#include <math.h>
#include "errors.h"
#include "uniscanner.h"


/**
 *	Emit an error from lexer
 *
 *	@param	lxr			Lexer structure
 *	@param	num			Error code
 */
static void lexer_error(lexer *const lxr, error_t num, ...)
{
	lxr->was_error = 1;

	va_list args;
	va_start(args, num);

	verror(lxr->io, num, args);

	va_end(args);
}

/**
 *	Scan next character from io
 *
 *	@param	lxr			Lexer structure
 *
 *	@return	Read character
 */
static char32_t scan(lexer *const lxr)
{
	lxr->character = uni_scan_char(lxr->io);
	return lxr->character;
}

/**
 *	Peek next character from io
 *
 *	@param	lxr			Lexer structure
 *
 *	@return	Peeked character
 */
static char32_t lookahead(lexer *const lxr)
{
	const size_t position = in_get_position(lxr->io);
	const char32_t result = uni_scan_char(lxr->io);
	in_set_position(lxr->io, position);
	return result;
}

/**
 *	Skip over a series of whitespace characters
 *
 *	@param	lxr			Lexer structure
 */
static void skip_whitespace(lexer *const lxr)
{
	while (lxr->character == '\n' || lxr->character == '\r'
		|| lxr->character == '\t' || lxr->character == ' ')
	{
		scan(lxr);
	}
}

/**
 *	Skip until we find the newline character that terminates the comment
 *
 *	@param	lxr			Lexer structure
 */
static void skip_line_comment(lexer *const lxr)
{
	while (lxr->character != '\n' && lxr->character != (char32_t)EOF)
	{
		scan(lxr);
	}
}

/**
 *	Skip until we find '*' and '/' that terminates the comment
 *
 *	@param	lxr			Lexer structure
 */
static void skip_block_comment(lexer *const lxr)
{
	while (lxr->character != '*' && scan(lxr) != '/')
	{
		if (lxr->character == (char32_t)EOF)
		{
			lexer_error(lxr, unterminated_block_comment);
			return;
		}
		scan(lxr);
	}
	scan(lxr);
}

/**
 *	Lex identifier or keyword [C99 6.4.1 & 6.4.2]
 *
 *	@param	lxr			Lexer structure
 *
 *	@return	keyword number on keyword, @c identifier on identifier
 */
static token_t lex_identifier_or_keyword(lexer *const lxr)
{
	char32_t spelling[MAXSTRINGL];
	size_t length = 0;

	do
	{
		spelling[length++] = lxr->character;
		scan(lxr);
	} while (utf8_is_letter(lxr->character) || utf8_is_digit(lxr->character));
	spelling[length] = '\0';

	const size_t repr = repr_reserve(lxr->sx, spelling);
	const item_t ref = repr_get_reference(lxr->sx, repr);
	if (ref >= 0 && repr != ITEM_MAX)
	{
		lxr->repr = repr;
		return TOK_IDENTIFIER;
	}
	else
	{
		return (token_t)ref;
	}
}

/**
 *	Lex numeric constant [C99 6.4.4.1 & 6.4.4.2]
 *
 *	@param	lxr			Lexer structure
 *
 *	@return	@c int_constant on integer, @c float_constant on floating point
 */
static token_t lex_numeric_constant(lexer *const lxr)
{
	int num_int = 0;
	double num_double = 0.0;
	int flag_int = 1;
	int flag_too_long = 0;

	while (utf8_is_digit(lxr->character))
	{
		num_int = num_int * 10 + (lxr->character - '0');
		num_double = num_double * 10 + (lxr->character - '0');
		scan(lxr);
	}

	if (num_double > (double)INT_MAX)
	{
		flag_too_long = 1;
		flag_int = 0;
	}

	if (lxr->character == '.')
	{
		flag_int = 0;
		double position_mult = 0.1;
		while (utf8_is_digit(scan(lxr)))
		{
			num_double += (lxr->character - '0') * position_mult;
			position_mult *= 0.1;
		}
	}

	if (utf8_is_power(lxr->character))
	{
		int power = 0;
		int sign = 1;
		scan(lxr);

		if (lxr->character == '-')
		{
			flag_int = 0;
			scan(lxr);
			sign = -1;
		}
		else if (lxr->character == '+')
		{
			scan(lxr);
		}

		if (!utf8_is_digit(lxr->character))
		{
			lexer_error(lxr, must_be_digit_after_exp);
			return TOK_FLOAT_CONST;
		}

		while (utf8_is_digit(lxr->character))
		{
			power = power * 10 + (lxr->character - '0');
			scan(lxr);
		}

		if (flag_int)
		{
			for (int i = 1; i <= power; i++)
			{
				num_int *= 10;
			}
		}
		num_double *= pow(10.0, sign * power);
	}

	if (flag_int)
	{
		lxr->num = num_int;
		return TOK_INT_CONST;
	}
	else
	{
		lxr->num_double = num_double;
		if (flag_too_long)
		{
			warning(lxr->io, too_long_int);
		}
		return TOK_FLOAT_CONST;
	}
}

/**	Get character or escape sequence after '\' */
static char32_t get_next_string_elem(lexer *const lxr)
{
	if (lxr->character == '\\')
	{
		switch (scan(lxr))
		{
			case 'n':
			case U'н':
				return '\n';

			case 't':
			case U'т':
				return '\t';

			case '0':
				return '\0';

			case '\\':
			case '\'':
			case '\"':
				return lxr->character;

			default:
				lexer_error(lxr, unknown_escape_sequence);
				return lxr->character;
		}
	}
	else
	{
		return lxr->character;
	}
}

/**
 *	Lex character constant [C99 6.4.4.4]
 *	@note Lexes the remainder of a character constant after apostrophe
 *
 *	@param	lxr			Lexer structure
 *
 *	@return	@c char_constant
 */
static token_t lex_char_constant(lexer *const lxr)
{
	if (scan(lxr) == '\'')
	{
		lexer_error(lxr, empty_character);
		lxr->num = 0;
		return TOK_CHAR_CONST;
	}

	lxr->num = get_next_string_elem(lxr);

	if (scan(lxr) == '\'')
	{
		scan(lxr);
	}
	else
	{
		lexer_error(lxr, expected_apost_after_char_const);
	}
	return TOK_CHAR_CONST;
}

/**
 *	Lex string literal [C99 6.4.5]
 *	@note	Lexes the remainder of a string literal after quote mark
 *
 *	@param	lxr			Lexer structure
 *
 *	@return	@c string_literal
 */
static token_t lex_string_literal(lexer *const lxr)
{
	size_t length = 0;
	int flag_too_long_string = 0;
	while (lxr->character == '\"')
	{
		scan(lxr);
		while (lxr->character != '"' && lxr->character != '\n' && length < MAXSTRINGL)
		{
			if (!flag_too_long_string)
			{
				lxr->lexstr[length++] = get_next_string_elem(lxr);
			}
			scan(lxr);
		}
		if (length == MAXSTRINGL)
		{
			lexer_error(lxr, string_too_long);
			flag_too_long_string = 1;
			while (lxr->character != '"' && lxr->character != '\n')
			{
				scan(lxr);
			}
		}
		if (lxr->character == '"')
		{
			scan(lxr);
		}
		else
		{
			lexer_error(lxr, missing_terminating_quote_char);
		}
		skip_whitespace(lxr);
	}
	lxr->num = (int)length;
	return TOK_STRING;
}


/*
 *	 __     __   __     ______   ______     ______     ______   ______     ______     ______
 *	/\ \   /\ "-.\ \   /\__  _\ /\  ___\   /\  == \   /\  ___\ /\  __ \   /\  ___\   /\  ___\
 *	\ \ \  \ \ \-.  \  \/_/\ \/ \ \  __\   \ \  __<   \ \  __\ \ \  __ \  \ \ \____  \ \  __\
 *	 \ \_\  \ \_\\"\_\    \ \_\  \ \_____\  \ \_\ \_\  \ \_\    \ \_\ \_\  \ \_____\  \ \_____\
 *	  \/_/   \/_/ \/_/     \/_/   \/_____/   \/_/ /_/   \/_/     \/_/\/_/   \/_____/   \/_____/
 */


lexer create_lexer(universal_io *const io, syntax *const sx)
{
	lexer lxr;
	lxr.io = io;
	lxr.sx = sx;
	lxr.repr = 0;

	lxr.was_error = 0;

	scan(&lxr);

	return lxr;
}


token_t lex(lexer *const lxr)
{
	if (lxr == NULL)
	{
		return TOK_EOF;
	}

	skip_whitespace(lxr);
	switch (lxr->character)
	{
		case (char32_t)EOF:
			return TOK_EOF;

		default:
			if (utf8_is_letter(lxr->character) || lxr->character == '#')
			{
				// Keywords [C99 6.4.1]
				// Identifiers [C99 6.4.2]
				return lex_identifier_or_keyword(lxr);
			}
			else
			{
				lexer_error(lxr, bad_character, lxr->character);
				// Pretending the character didn't exist
				scan(lxr);
				return lex(lxr);
			}

		// Integer Constants [C99 6.4.4.1]
		// Floating Constants [C99 6.4.4.2]
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			return lex_numeric_constant(lxr);

		// Character Constants [C99 6.4.4.4]
		case '\'':
			return lex_char_constant(lxr);

		// String Literals [C99 6.4.5]
		case '\"':
			return lex_string_literal(lxr);

		// Punctuators [C99 6.4.6]
		case '?':
			scan(lxr);
			return TOK_QUESTION;

		case '[':
			scan(lxr);
			return TOK_LSQUARE;

		case ']':
			scan(lxr);
			return TOK_RSQUARE;

		case '(':
			scan(lxr);
			return TOK_LPAREN;

		case ')':
			scan(lxr);
			return TOK_RPAREN;

		case '{':
			scan(lxr);
			return TOK_LBRACE;

		case '}':
			scan(lxr);
			return TOK_RBRACE;

		case '~':
			scan(lxr);
			return TOK_TILDE;

		case ':':
			scan(lxr);
			return TOK_COLON;

		case ';':
			scan(lxr);
			return TOK_SEMICOLON;

		case ',':
			scan(lxr);
			return TOK_COMMA;

		case '.':
			if (utf8_is_digit(lookahead(lxr)))
			{
				return lex_numeric_constant(lxr);
			}
			else
			{
				scan(lxr);
				return TOK_PERIOD;
			}

		case '*':
			if (scan(lxr) == '=')
			{
				scan(lxr);
				return TOK_STAREQUAL;
			}
			else
			{
				return TOK_STAR;
			}

		case '!':
			if (scan(lxr) == '=')
			{
				scan(lxr);
				return TOK_EXCLAIMEQUAL;
			}
			else
			{
				return TOK_EXCLAIM;
			}

		case '%':
			if (scan(lxr) == '=')
			{
				scan(lxr);
				return TOK_PERCENTEQUAL;
			}
			else
			{
				return TOK_PERCENT;
			}

		case '^':
			if (scan(lxr) == '=')
			{
				scan(lxr);
				return TOK_CARETEQUAL;
			}
			else
			{
				return TOK_CARET;
			}

		case '=':
			if (scan(lxr) == '=')
			{
				scan(lxr);
				return TOK_EQUALEQUAL;
			}
			else
			{
				return TOK_EQUAL;
			}

		case '+':
			switch (scan(lxr))
			{
				case '=':
					scan(lxr);
					return TOK_PLUSEQUAL;

				case '+':
					scan(lxr);
					return TOK_PLUSPLUS;

				default:
					return TOK_PLUS;
			}

		case '|':
			switch (scan(lxr))
			{
				case '=':
					scan(lxr);
					return TOK_PIPEEQUAL;

				case '|':
					scan(lxr);
					return TOK_PIPEPIPE;

				default:
					return TOK_PIPE;
			}

		case '&':
			switch (scan(lxr))
			{
				case '=':
					scan(lxr);
					return TOK_AMPEQUAL;

				case '&':
					scan(lxr);
					return TOK_AMPAMP;

				default:
					return TOK_AMP;
			}

		case '-':
			switch (scan(lxr))
			{
				case '=':
					scan(lxr);
					return TOK_MINUSEQUAL;

				case '-':
					scan(lxr);
					return TOK_MINUSMINUS;

				case '>':
					scan(lxr);
					return TOK_ARROW;

				default:
					return TOK_MINUS;
			}

		case '<':
			switch (scan(lxr))
			{
				case '<':
					if (scan(lxr) == '=')
					{
						scan(lxr);
						return TOK_LESSLESSEQUAL;
					}
					else
					{
						return TOK_LESSLESS;
					}

				case '=':
					scan(lxr);
					return TOK_LESSEQUAL;

				default:
					return TOK_LESS;
			}

		case '>':
			switch (scan(lxr))
			{
				case '>':
					if (scan(lxr) == '=')
					{
						scan(lxr);
						return TOK_GREATERGREATEREQUAL;
					}
					else
					{
						return TOK_GREATERGREATER;
					}

				case '=':
					scan(lxr);
					return TOK_GREATEREQUAL;

				default:
					return TOK_GREATER;
			}

		case '/':
			switch (scan(lxr))
			{
				case '=':
					scan(lxr);
					return TOK_SLASHEQUAL;

				// Comments [C99 6.4.9]
				case '/':
					skip_line_comment(lxr);
					return lex(lxr);

				case '*':
					skip_block_comment(lxr);
					return lex(lxr);

				default:
					return TOK_SLASH;
			}
	}
}

token_t peek(lexer *const lxr)
{
	const size_t position = in_get_position(lxr->io);
	const char32_t character = lxr->character;
	const token_t peek_token = lex(lxr);
	lxr->character = character;
	in_set_position(lxr->io, position);
	return peek_token;
}
