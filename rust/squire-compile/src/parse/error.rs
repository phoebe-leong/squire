use squire_runtime::value::numeral::NumeralParseError;
use super::{Token, TokenKind};

#[derive(Debug, Clone, PartialEq, Eq)]
#[non_exhaustive]
pub enum ErrorKind {
	BadNumeral(NumeralParseError),
	BadFrakturSuffix,
	UnterminatedEscapeSequence,
	InvalidHexDigit(char),
	InvalidHexEscape(u32),
	UnknownEscapeCharacter(char),
	UnknownMacroInvocation(String),
	UnknownTokenStart(char),
	MissingRequiredAst(&'static str),
	BadToken { given: Option<Token>, expected: Vec<TokenKind> },
	Message(&'static str),
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Error {
	pub lineno: usize,
	pub file: Option<String>,
	pub error: ErrorKind
}

pub type Result<T> = std::result::Result<T, Error>;


impl From<NumeralParseError> for ErrorKind {
	#[inline]
	fn from(error: NumeralParseError) -> Self {
		Self::BadNumeral(error)
	}
}

impl From<&'static str> for ErrorKind {
	#[inline]
	fn from(msg: &'static str) -> Self {
		Self::Message(msg)
	}
}