use crate::ast::{Expression, expression::Primary, Statements};
use crate::value::{Value, Journey};
use crate::parse::{Parser, Parsable, Error as ParseError};
use crate::parse::token::{Token, TokenKind, Keyword, Symbol, ParenKind};
use crate::compile::{Compiler, Compilable, Target, Globals, Error as CompileError};

pub type Type = Primary;

#[derive(Debug)]
pub struct Argument {
	name: String,
	kind: Option<Type>,
	default: Option<Expression>
}

#[derive(Debug)]
pub struct Arguments {
	normal: Vec<Argument>,
	vararg: Option<String>,
	varkwarg: Option<String>,
	return_type: Option<Type>
}

#[derive(Debug)]
pub struct Function {
	name: String,
	args: Arguments,
	body: Statements
}

impl Parsable for Arguments {
	const TYPE_NAME: &'static str = "<arguments>";

	fn parse<I: Iterator<Item=char>>(parser: &mut Parser<'_, I>) -> Result<Option<Self>, ParseError> {
		if parser.guard(TokenKind::LeftParen(ParenKind::Round))?.is_none() {
			return Ok(None);
		}

		let mut arguments = Self {
			normal: Vec::new(),
			vararg: None,
			varkwarg: None,
			return_type: None
		};

		while let Some(name) = parser.guard_identifier()? {
			let mut argument = Argument { name, kind: None, default: None };

			if parser.guard(TokenKind::Symbol(Symbol::Colon))?.is_some() {
				argument.kind = Some(Type::expect_parse(parser)?);
			}

			if parser.guard(TokenKind::Symbol(Symbol::Equal))?.is_some() {
				argument.default = Some(Expression::expect_parse(parser)?);
			}

			arguments.normal.push(argument);

			if parser.guard(TokenKind::Symbol(Symbol::Comma))?.is_none() {
				break;
			}
		}

		if parser.guard(TokenKind::Symbol(Symbol::Asterisk))?.is_some() {
			arguments.vararg = Some(parser.expect_identifier()?);

			if let Token::RightParen(_) = parser.expect([
				TokenKind::Symbol(Symbol::Comma),
				TokenKind::RightParen(ParenKind::Round)
			])? {
				parser.undo_next_token();
			}
		}

		if parser.guard(TokenKind::Symbol(Symbol::AsteriskAsterisk))?.is_some() {
			arguments.varkwarg = Some(parser.expect_identifier()?);

			parser.guard(TokenKind::Symbol(Symbol::Comma))?;
		}

		parser.expect(TokenKind::RightParen(ParenKind::Round))?;

		if parser.guard(TokenKind::Symbol(Symbol::Colon))?.is_some() {
			arguments.return_type = Some(Type::expect_parse(parser)?);
		}

		Ok(Some(arguments))
	}
}

impl Parsable for Function {
	const TYPE_NAME: &'static str = Keyword::Function.repr();

	fn parse<I: Iterator<Item=char>>(parser: &mut Parser<'_, I>) -> Result<Option<Self>, ParseError> {
		if parser.guard(TokenKind::Keyword(Keyword::Function))?.is_none() {
			return Ok(None);
		}

		Self::parse_without_keyword(parser).map(Some)
	}
}

impl Function {
	pub fn parse_without_keyword<I: Iterator<Item=char>>(parser: &mut Parser<'_, I>) -> Result<Self, ParseError> {
		let name = parser.expect_identifier()?;
		let args = Arguments::expect_parse(parser)?;
		let body  = Statements::expect_parse(parser)?;

		Ok(Self { name, args, body })
	}

	pub fn build_journey(self, globals: Globals) -> Result<Journey, CompileError> {
		use crate::runtime::Opcode;

		let mut body_compiler = Compiler::with_globals(globals);

		if self.args.vararg.is_some() || self.args.varkwarg.is_some() || self.args.return_type.is_some() {
			todo!();
		}

		let mut arg_names = Vec::new();
		for arg in self.args.normal {
			if arg.kind.is_some() || arg.default.is_some() {
				todo!();
			}

			arg_names.push(arg.name.clone());
			body_compiler.define_local(arg.name);
		}

		let return_target = body_compiler.next_target();
		self.body.compile(&mut body_compiler, Some(return_target))?;
		// todo: what if `return_target` is just used for scratch?.
		body_compiler.opcode(Opcode::Return);
		body_compiler.target(return_target);

		Ok(Journey::new(self.name.clone(), false, arg_names, body_compiler.finish()))
	}
}

impl Compilable for Function {
	fn compile(self, compiler: &mut Compiler, target: Option<Target>) -> Result<(), CompileError> {
		use crate::runtime::Opcode;

		let name = self.name.clone();
		let journey = Value::Journey(self.build_journey(compiler.globals().clone())?.into());
		let global = compiler.define_global(name, Some(journey))?;

		if let Some(target) = target {
			compiler.opcode(Opcode::LoadGlobal);
			compiler.global(global);
			compiler.target(target);
		}

		Ok(())
	}
}
