//
//

#include "DynamicSEXP.h"

namespace sexp {

DynamicSEXP::DynamicSEXP(const std::string& name) : _name(name) {
}
const std::string& DynamicSEXP::getName() const {
	return _name;
}
const std::string& DynamicSEXP::getHelpText() const {
	return _help_text;
}

}

