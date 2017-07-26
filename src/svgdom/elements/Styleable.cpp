#include "Styleable.hpp"

#include <set>
#include <iomanip>
#include <cctype>
#include <array>

#include <utki/debug.hpp>
#include <utki/util.hpp>

#include "../util.hxx"
#include "Element.hpp"

using namespace svgdom;


void Styleable::attribsToStream(std::ostream& s) const{
	if(this->styles.size() == 0){
		return;
	}
	
	s << " style=\"";
	
	bool isFirst = true;
	
	for(auto& st : this->styles){
		if(isFirst){
			isFirst = false;
		}else{
			s << "; ";
		}
		
		ASSERT(st.first != StyleProperty_e::UNKNOWN)
		
		s << propertyToString(st.first) << ":";
		
		switch(st.first){
			default:
				ASSERT(false)
				break;
			case StyleProperty_e::STOP_OPACITY:
			case StyleProperty_e::OPACITY:
			case StyleProperty_e::STROKE_OPACITY:
			case StyleProperty_e::FILL_OPACITY:
				s << st.second.opacity;
				break;
			case StyleProperty_e::STOP_COLOR:
			case StyleProperty_e::FILL:
			case StyleProperty_e::STROKE:
				s << st.second.paintToString();
				break;
			case StyleProperty_e::STROKE_WIDTH:
				s << st.second.length;
				break;
			case StyleProperty_e::STROKE_LINECAP:
				switch(st.second.strokeLineCap){
					default:
						ASSERT(false)
						break;
					case StrokeLineCap_e::BUTT:
						s << "butt";
						break;
					case StrokeLineCap_e::ROUND:
						s << "round";
						break;
					case StrokeLineCap_e::SQUARE:
						s << "square";
						break;
				}
				break;
			case StyleProperty_e::STROKE_LINEJOIN:
				switch(st.second.strokeLineJoin){
					default:
						ASSERT(false)
						break;
					case StrokeLineJoin_e::MITER:
						s << "miter";
						break;
					case StrokeLineJoin_e::ROUND:
						s << "round";
						break;
					case StrokeLineJoin_e::BEVEL:
						s << "bevel";
						break;
				}
				break;
			case StyleProperty_e::FILL_RULE:
				switch(st.second.fillRule){
					default:
						ASSERT(false)
						break;
					case FillRule_e::EVENODD:
						s << "evenodd";
						break;
					case FillRule_e::NONZERO:
						s << "nonzero";
						break;
				}
				break;
		}
	}
	
	s << "\"";
}

//input parameter 'str' should have no leading or trailing white spaces
StylePropertyValue Styleable::parseStylePropertyValue(StyleProperty_e type, const std::string& str){
	StylePropertyValue v;

	if (str == "inherit") {
		v.type = StylePropertyValue::Type_e::INHERIT;
		return v;
	}

	switch(type){
		default:
			ASSERT(false)
			break;
		case StyleProperty_e::STOP_OPACITY:
		case StyleProperty_e::OPACITY:
		case StyleProperty_e::STROKE_OPACITY:
		case StyleProperty_e::FILL_OPACITY:
			{
				std::istringstream iss(str);
				v.opacity = readInReal(iss);
				utki::clampRange(v.opacity, real(0), real(1));
			}
			break;
		case StyleProperty_e::STOP_COLOR:
		case StyleProperty_e::FILL:
		case StyleProperty_e::STROKE:
			v = StylePropertyValue::parsePaint(str);
//				TRACE(<< "paint read = " << std::hex << v.integer << std::endl)
			break;
		case StyleProperty_e::STROKE_WIDTH:
			v.length = Length::parse(str);
//				TRACE(<< "stroke-width read = " << v.length << std::endl)
			break;
		case StyleProperty_e::STROKE_LINECAP:
			if(str == "butt"){
				v.strokeLineCap = StrokeLineCap_e::BUTT;
			}else if(str == "round"){
				v.strokeLineCap = StrokeLineCap_e::ROUND;
			}else if(str == "square"){
				v.strokeLineCap = StrokeLineCap_e::SQUARE;
			}else{
				TRACE(<< "unknown strokeLineCap value:" << str << std::endl)
			}
			break;
		case StyleProperty_e::STROKE_LINEJOIN:
			if(str == "miter"){
				v.strokeLineJoin = StrokeLineJoin_e::MITER;
			}else if(str == "round"){
				v.strokeLineJoin = StrokeLineJoin_e::ROUND;
			}else if(str == "bevel"){
				v.strokeLineJoin = StrokeLineJoin_e::BEVEL;
			}else{
				TRACE(<< "unknown strokeLineJoin value:" << str << std::endl)
			}
			break;
		case StyleProperty_e::FILL_RULE:
			if(str == "nonzero"){
				v.fillRule = FillRule_e::NONZERO;
			}else if(str == "evenodd"){
				v.fillRule = FillRule_e::EVENODD;
			}else{
				TRACE(<< "unknown fill-rule value:" << str << std::endl)
			}
			break;
	}
	return v;
}


namespace{
StylePropertyValue parseStylePropertyValue(StyleProperty_e type, std::istream& s){
	skipWhitespaces(s);
	std::string str = readTillChar(s, ';');
	str = trimTail(str);
	return Styleable::parseStylePropertyValue(type, str);
}
}

decltype(Styleable::styles) Styleable::parse(const std::string& str){
	std::istringstream s(str);
	
	s >> std::skipws;
	
	decltype(Styleable::styles) ret;
	
	while(!s.eof()){
		skipWhitespaces(s);
		std::string property = readTillCharOrWhitespace(s, ':');
		
		StyleProperty_e type = Styleable::stringToProperty(property);
		
		if(type == StyleProperty_e::UNKNOWN){
			//unknown style property, skip it
			TRACE(<< "Unknown style property: " << property << std::endl)
			skipTillCharInclusive(s, ';');
			continue;
		}
		
		if(s.get() != ':'){
			return ret;//expected colon
		}
		
		StylePropertyValue v = ::parseStylePropertyValue(type, s);
		
		skipWhitespaces(s);
		
		if(!s.eof() && s.get() != ';'){
			return ret;//expected semicolon
		}
		
		ret[type] = std::move(v);
	}
	
	return ret;
}

namespace{
const std::set<StyleProperty_e> nonInheritedStyleProperties = {
	StyleProperty_e::ALIGNMENT_BASELINE,
	StyleProperty_e::BASELINE_SHIFT,
	StyleProperty_e::CLIP,
	StyleProperty_e::CLIP_PATH,
	StyleProperty_e::DISPLAY,
	StyleProperty_e::DOMINANT_BASELINE,
	StyleProperty_e::ENABLE_BACKGROUND,
	StyleProperty_e::FILTER,
	StyleProperty_e::FLOOD_COLOR,
	StyleProperty_e::FLOOD_OPACITY,
	StyleProperty_e::LIGHTING_COLOR,
	StyleProperty_e::MASK,
	StyleProperty_e::OPACITY,
	StyleProperty_e::OVERFLOW,
	StyleProperty_e::STOP_COLOR,
	StyleProperty_e::STOP_OPACITY,
	StyleProperty_e::TEXT_DECORATION,
	StyleProperty_e::UNICODE_BIDI
};
}

bool Styleable::isStylePropertyInherited(StyleProperty_e p) {
	return nonInheritedStyleProperties.find(p) == nonInheritedStyleProperties.end();
}

StyleProperty_e Styleable::stringToProperty(std::string str){
	if(str == "fill"){
		return StyleProperty_e::FILL;
	}else if(str == "fill-opacity"){
		return StyleProperty_e::FILL_OPACITY;
	}else if(str == "stroke"){
		return StyleProperty_e::STROKE;
	}else if(str == "stroke-width"){
		return StyleProperty_e::STROKE_WIDTH;
	}else if(str == "stroke-linecap"){
		return StyleProperty_e::STROKE_LINECAP;
	}else if(str == "stroke-linejoin"){
		return StyleProperty_e::STROKE_LINEJOIN;
	}else if(str == "stroke-opacity"){
		return StyleProperty_e::STROKE_OPACITY;
	}else if(str == "opacity"){
		return StyleProperty_e::OPACITY;
	}else if(str == "stop-opacity"){
		return StyleProperty_e::STOP_OPACITY;
	}else if(str == "stop-color"){
		return StyleProperty_e::STOP_COLOR;
	}else if(str == "fill-rule"){
		return StyleProperty_e::FILL_RULE;
	}
	
	return StyleProperty_e::UNKNOWN;
}

std::string Styleable::propertyToString(StyleProperty_e p){
	switch(p){
		default:
			ASSERT(false)
			return "";
		case StyleProperty_e::FILL:
			return "fill";
		case StyleProperty_e::FILL_OPACITY:
			return "fill-opacity";
		case StyleProperty_e::STROKE:
			return "stroke";
		case StyleProperty_e::STROKE_WIDTH:
			return "stroke-width";
		case StyleProperty_e::STROKE_LINECAP:
			return "stroke-linecap";
		case StyleProperty_e::STROKE_LINEJOIN:
			return "stroke-linejoin";
		case StyleProperty_e::STROKE_OPACITY:
			return "stroke-opacity";
		case StyleProperty_e::OPACITY:
			return "opacity";
		case StyleProperty_e::STOP_OPACITY:
			return "stop-opacity";
		case StyleProperty_e::STOP_COLOR:
			return "stop-color";
		case StyleProperty_e::FILL_RULE:
			return "fill-rule";
	}
}

const StylePropertyValue* Styleable::findStyleProperty(StyleProperty_e p)const{
	auto i = this->styles.find(p);
	if(i != this->styles.end()){
		return &i->second;
	}
	return nullptr;
}

Rgb StylePropertyValue::getRgb() const{
	auto c = this->color;
	
	Rgb ret;
	
	ret.r = real(c & 0xff) / real(0xff);
	ret.g = real((c >> 8) & 0xff) / real(0xff);
	ret.b = real((c >> 16) & 0xff) / real(0xff);
	
	return ret;
}

namespace{
const std::map<std::string, std::uint32_t> colorNames = {
	{"aliceblue", 0xfff8f0},
	{"antiquewhite", 0xd7ebfa},
	{"aqua", 0xffff00},
	{"aquamarine", 0xd4ff7f},
	{"azure", 0xfffff0},
	{"beige", 0xdcf5f5},
	{"bisque", 0xc4e4ff},
	{"black", 0x0},
	{"blanchedalmond", 0xcdebff},
	{"blue", 0xff0000},
	{"blueviolet", 0xe22b8a},
	{"brown", 0x2a2aa5},
	{"burlywood", 0x87b8de},
	{"cadetblue", 0xa09e5f},
	{"chartreuse", 0xff7f},
	{"chocolate", 0x1e69d2},
	{"coral", 0x507fff},
	{"cornflowerblue", 0xed9564},
	{"cornsilk", 0xdcf8ff},
	{"crimson", 0x3c14dc},
	{"cyan", 0xffff00},
	{"darkblue", 0x8b0000},
	{"darkcyan", 0x8b8b00},
	{"darkgoldenrod", 0xb86b8},
	{"darkgray", 0xa9a9a9},
	{"darkgreen", 0x6400},
	{"darkgrey", 0xa9a9a9},
	{"darkkhaki", 0x6bb7bd},
	{"darkmagenta", 0x8b008b},
	{"darkolivegreen", 0x2f6b55},
	{"darkorange", 0x8cff},
	{"darkorchid", 0xcc3299},
	{"darkred", 0x8b},
	{"darksalmon", 0x7a96e9},
	{"darkseagreen", 0x8fbc8f},
	{"darkslateblue", 0x8b3d48},
	{"darkslategray", 0x4f4f2f},
	{"darkslategrey", 0x4f4f2f},
	{"darkturquoise", 0xd1ce00},
	{"darkviolet", 0xd30094},
	{"deeppink", 0x9314ff},
	{"deepskyblue", 0xffbf00},
	{"dimgray", 0x696969},
	{"dimgrey", 0x696969},
	{"dodgerblue", 0xff901e},
	{"firebrick", 0x2222b2},
	{"floralwhite", 0xf0faff},
	{"forestgreen", 0x228b22},
	{"fuchsia", 0xff00ff},
	{"gainsboro", 0xdcdcdc},
	{"ghostwhite", 0xfff8f8},
	{"gold", 0xd7ff},
	{"goldenrod", 0x20a5da},
	{"gray", 0x808080},
	{"grey", 0x808080},
	{"green", 0x8000},
	{"greenyellow", 0x2fffad},
	{"honeydew", 0xf0fff0},
	{"hotpink", 0xb469ff},
	{"indianred", 0x5c5ccd},
	{"indigo", 0x82004b},
	{"ivory", 0xf0ffff},
	{"khaki", 0x8ce6f0},
	{"lavender", 0xfae6e6},
	{"lavenderblush", 0xf5f0ff},
	{"lawngreen", 0xfc7c},
	{"lemonchiffon", 0xcdfaff},
	{"lightblue", 0xe6d8ad},
	{"lightcoral", 0x8080f0},
	{"lightcyan", 0xffffe0},
	{"lightgoldenrodyellow", 0xd2fafa},
	{"lightgray", 0xd3d3d3},
	{"lightgreen", 0x90ee90},
	{"lightgrey", 0xd3d3d3},
	{"lightpink", 0xc1b6ff},
	{"lightsalmon", 0x7aa0ff},
	{"lightseagreen", 0xaab220},
	{"lightskyblue", 0xface87},
	{"lightslategray", 0x998877},
	{"lightslategrey", 0x998877},
	{"lightsteelblue", 0xdec4b0},
	{"lightyellow", 0xe0ffff},
	{"lime", 0xff00},
	{"limegreen", 0x32cd32},
	{"linen", 0xe6f0fa},
	{"magenta", 0xff00ff},
	{"maroon", 0x80},
	{"mediumaquamarine", 0xaacd66},
	{"mediumblue", 0xcd0000},
	{"mediumorchid", 0xd355ba},
	{"mediumpurple", 0xdb7093},
	{"mediumseagreen", 0x71b33c},
	{"mediumslateblue", 0xee687b},
	{"mediumspringgreen", 0x9afa00},
	{"mediumturquoise", 0xccd148},
	{"mediumvioletred", 0x8515c7},
	{"midnightblue", 0x701919},
	{"mintcream", 0xfafff5},
	{"mistyrose", 0xe1e4ff},
	{"moccasin", 0xb5e4ff},
	{"navajowhite", 0xaddeff},
	{"navy", 0x800000},
	{"oldlace", 0xe6f5fd},
	{"olive", 0x8080},
	{"olivedrab", 0x238e6b},
	{"orange", 0xa5ff},
	{"orangered", 0x45ff},
	{"orchid", 0xd670da},
	{"palegoldenrod", 0xaae8ee},
	{"palegreen", 0x98fb98},
	{"paleturquoise", 0xeeeeaf},
	{"palevioletred", 0x9370db},
	{"papayawhip", 0xd5efff},
	{"peachpuff", 0xb9daff},
	{"peru", 0x3f85cd},
	{"pink", 0xcbc0ff},
	{"plum", 0xdda0dd},
	{"powderblue", 0xe6e0b0},
	{"purple", 0x800080},
	{"red", 0xff},
	{"rosybrown", 0x8f8fbc},
	{"royalblue", 0xe16941},
	{"saddlebrown", 0x13458b},
	{"salmon", 0x7280fa},
	{"sandybrown", 0x60a4f4},
	{"seagreen", 0x578b2e},
	{"seashell", 0xeef5ff},
	{"sienna", 0x2d52a0},
	{"silver", 0xc0c0c0},
	{"skyblue", 0xebce87},
	{"slateblue", 0xcd5a6a},
	{"slategray", 0x908070},
	{"slategrey", 0x908070},
	{"snow", 0xfafaff},
	{"springgreen", 0x7fff00},
	{"steelblue", 0xb48246},
	{"tan", 0x8cb4d2},
	{"teal", 0x808000},
	{"thistle", 0xd8bfd8},
	{"tomato", 0x4763ff},
	{"turquoise", 0xd0e040},
	{"violet", 0xee82ee},
	{"wheat", 0xb3def5},
	{"white", 0xffffff},
	{"whitesmoke", 0xf5f5f5},
	{"yellow", 0xffff},
	{"yellowgreen", 0x32cd9a}
};

}//~namespace

//'str' should have no leading and/or trailing white spaces.
StylePropertyValue StylePropertyValue::parsePaint(const std::string& str){
	StylePropertyValue ret;
	
	if(str.size() == 0){
		ret.type = StylePropertyValue::Type_e::NONE;
		return ret;
	}
	
	ASSERT(!std::isspace(str[0])) //leading spaces should be skept already	
	
	{
		std::string url = "url(";
		if(url == str.substr(0, url.length())){
			std::istringstream s(str);
			
			std::string tmpStr;
			s >> std::setw(int(url.length())) >> tmpStr >> std::setw(0);
			ASSERT(tmpStr == url)
			
			skipWhitespaces(s);
			tmpStr = readTillCharOrWhitespace(s, ')');
			
			skipWhitespaces(s);
			if(s.get() == ')'){
				ret.str = tmpStr;
				ret.type = StylePropertyValue::Type_e::URL;
				ret.url = nullptr;
				return ret;
			}
		}
	}
	
	if(str == "none"){
		ret.type = StylePropertyValue::Type_e::NONE;
		return ret;
	}
	
	if(str == "currentColor"){
		ret.type = StylePropertyValue::Type_e::CURRENT_COLOR;
		return ret;
	}
	
	//check if # notation
	if(str[0] == '#'){
		std::istringstream s(str.substr(1, 6));
		
		std::array<std::uint8_t, 6> d;
		unsigned numDigits = 0;
		for(auto i = d.begin(); i != d.end(); ++i, ++numDigits){
			char c = s.get();
			if('0' <= c && c <= '9'){
				(*i) = c - '0';
			}else if('a' <= c && c <= 'f'){
				(*i) = 10 + c - 'a';
			}else if('A' <= c && c <= 'F'){
				(*i) = 10 + c - 'A';
			}else{
				break;
			}
			
			ASSERT(((*i) & 0xf0) == 0) //only one hex digit
		}
		switch(numDigits){
			case 3:
				ret.color = (std::uint32_t(d[0]) << 4) | (std::uint32_t(d[0]))
						| (std::uint32_t(d[1]) << 12) | (std::uint32_t(d[1]) << 8)
						| (std::uint32_t(d[2]) << 20) | (std::uint32_t(d[2]) << 16);
				break;
			case 6:
				ret.color = (std::uint32_t(d[0]) << 4) | (std::uint32_t(d[1]))
						| (std::uint32_t(d[2]) << 12) | (std::uint32_t(d[3]) << 8)
						| (std::uint32_t(d[4]) << 20) | (std::uint32_t(d[5]) << 16);
				break;
			default:
				ret.type = StylePropertyValue::Type_e::NONE;
				break;
		}
		
//		TRACE(<< "# color read = " << std::hex << ret.integer << std::endl)
		return ret;
	}
	
	//check if rgb() or RGB() notation
	{
		const std::string rgb = "rgb(";
		if(rgb == str.substr(0, rgb.length())){
			std::istringstream s(str);
			
			std::string tmpStr;
			
			s >> std::setw(int(rgb.length())) >> tmpStr >> std::setw(0);
			ASSERT(tmpStr == rgb)
			
			std::uint32_t r, g, b;
			
			skipWhitespaces(s);
			s >> r;
			skipWhitespacesAndOrComma(s);
			s >> g;
			skipWhitespacesAndOrComma(s);
			s >> b;
			skipWhitespaces(s);
			
			if(s.get() == ')'){
				ret.color = r | (g << 8) | (b << 16);
			}
			return ret;
		}
	}
	
	//check if color name
	{
		std::istringstream s(str);
		std::string name;
		s >> name;
		
		auto i = colorNames.find(name);
		if(i != colorNames.end()){
			ASSERT(i->first == name)
			ret.str = name;
			ret.color = i->second;
			return ret;
		}
	}
	
	return ret;
}

std::string StylePropertyValue::paintToString()const{
	switch(this->type){
		default:
		case Type_e::NONE:
			return "none";
		case Type_e::INHERIT:
			return "inherit";
		case Type_e::CURRENT_COLOR:
			return "currentColor";
		case Type_e::NORMAL:
			if(this->str.size() == 0){
				//it is a # notation

				std::stringstream s;
				s << std::hex;
				s << "#";
				s << ((this->color >> 4) & 0xf);
				s << ((this->color) & 0xf);
				s << ((this->color >> 12) & 0xf);
				s << ((this->color >> 8) & 0xf);
				s << ((this->color >> 20) & 0xf);
				s << ((this->color >> 16) & 0xf);
				return s.str();
			}else{
				return this->str;
			}
		case Type_e::URL:
			if(!this->url){
				return "none";
			}
			{
				std::stringstream ss;
				ss << "url(#" << this->url->id << ")";
				return ss.str();
			}
	}
}
