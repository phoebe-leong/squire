use crate::vm::{Vm, Error as RuntimeError};
use crate::value::{Value, Numeral, Text};
use crate::value::ops::{ConvertTo, Matches, IsEqual, Compare, Dump};

pub type Veracity = bool;

impl Dump for Veracity {
	fn dump(&self, to: &mut String, _: &mut Vm) -> Result<(), RuntimeError> {
		if *self {
			to.push_str("yay");
		} else {
			to.push_str("nay");
		}

		Ok(())
	}
}


impl ConvertTo<Numeral> for Veracity {
	fn convert(&self, _: &mut Vm) -> Result<Numeral, RuntimeError> {
		Ok(Numeral::new(*self as i64))
	}
}

impl ConvertTo<Text> for Veracity {
	fn convert(&self, vm: &mut Vm) -> Result<Text, RuntimeError> {
		let mut text = String::with_capacity(3);

		self.dump(&mut text, vm)?;

		Ok(Text::new(text))
	}
}

impl Matches for Veracity {
	fn matches(&self, rhs: &Value, vm: &mut Vm) -> Result<bool, RuntimeError> {
		self.is_equal(rhs, vm)
	}
}

impl IsEqual for Veracity {
	fn is_equal(&self, rhs: &Value, _: &mut Vm) -> Result<bool, RuntimeError> {
		if let Value::Veracity(rhs) = rhs {
			Ok(*self == *rhs)
		} else {
			Ok(false)
		}
	}
}

impl Compare for Veracity {
	fn compare(&self, rhs: &Value, vm: &mut Vm) -> Result<Option<std::cmp::Ordering>, RuntimeError> {
		Ok(self.partial_cmp(&rhs.convert_to::<Self>(vm)?))
	}
}
