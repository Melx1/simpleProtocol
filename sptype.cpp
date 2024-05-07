#include "sptype.h"


Quality operator|(Quality lhs, Quality rhs) {
    using QualityType = std::underlying_type<Quality>::type;
    return Quality(static_cast<QualityType>(lhs) | static_cast<QualityType>(rhs));
}


Quality operator&(Quality lhs, Quality rhs) {
    using QualityType = std::underlying_type<Quality>::type;
    return Quality(static_cast<QualityType>(lhs) & static_cast<QualityType>(rhs));
}

std::ostream& operator<<(std::ostream& os, const Quality& obj) {
    std::string str = "[";
    std::string sep;
    if (static_cast<bool>(obj == Quality::Valid)) {
        str.append("Valid");
        sep = ", ";
    }
    if (static_cast<bool>(obj & Quality::Overflow)) {
        str.append(sep);
        str.append("Substituted");
        sep = ", ";
    }
    if (static_cast<bool>(obj & Quality::Substituted)) {
        str.append(sep);
        str.append("Overflow");
        sep = ", ";
    }
    str.append("]");
    os << str;

    return os;
}


std::ostream& operator<<(std::ostream& os, const DigitValue& value) {
    os << static_cast<int>(value);
}
