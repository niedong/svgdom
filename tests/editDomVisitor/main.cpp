#include "../../src/svgdom/dom.hpp"
#include "../../src/svgdom/StreamWriter.hpp"

#include <utki/debug.hpp>


//Visitor to remove all Line elements
class EditingVisitor : public svgdom::Visitor{
	std::vector<std::pair<svgdom::Container*, T_ChildIter>> elementsToRemove;
	
	void addToRemove(){
		if(!this->curParent()){
			//root element does not have parent, nowhere to remove it from
			return;
		}
		this->elementsToRemove.push_back(std::make_pair(this->curParent(), this->curIter()));
	}
	
public:
	void visit(svgdom::SvgElement& e) override{
		this->relayAccept(e);
	}
	
	void visit(svgdom::GElement& e) override{
		this->relayAccept(e);
	}
	
	void visit(svgdom::PathElement& e) override{
		e.id = "new id for path";
	}

	void visit(svgdom::LineElement& e) override{
		this->addToRemove();
	}
	
	void removeLines(){
		for(auto& p : this->elementsToRemove){
			ASSERT_ALWAYS(p.first)
			p.first->children.erase(p.second);
		}
	}
};

int main(int argc, char** argv){
	auto dom = utki::makeUnique<svgdom::SvgElement>();

	svgdom::PathElement path;

	svgdom::PathElement::Step step;

	step.type = svgdom::PathElement::Step::Type_e::MOVE_ABS;
	step.x = 0;
	step.y = 0;
	path.path.push_back(step);

	step.type = svgdom::PathElement::Step::Type_e::LINE_ABS;
	step.x = 0;
	step.y = 300;
	path.path.push_back(step);

	dom->children.push_back(utki::makeUnique<svgdom::PathElement>(path));

	dom->children.push_back(utki::makeUnique<svgdom::LineElement>());
	
	{
		auto g = utki::makeUnique<svgdom::GElement>();
		g->children.push_back(utki::makeUnique<svgdom::LineElement>());
		
		dom->children.push_back(std::move(g));
	}
	
	EditingVisitor visitor;
	
	dom->accept(visitor);
	
	ASSERT_ALWAYS(dom->children.size() == 3)
	ASSERT_ALWAYS(dynamic_cast<svgdom::PathElement*>(dom->children.begin()->get()))
	ASSERT_ALWAYS(dynamic_cast<svgdom::LineElement*>((++dom->children.begin())->get()))
	ASSERT_ALWAYS(dynamic_cast<svgdom::GElement*>((++++dom->children.begin())->get()))
	ASSERT_ALWAYS(dynamic_cast<svgdom::GElement*>((++++dom->children.begin())->get())->children.size() == 1)
	ASSERT_ALWAYS(
			dynamic_cast<svgdom::LineElement*>(dynamic_cast<svgdom::GElement*>((++++dom->children.begin())->get())->children.front().get())
		)
	
	visitor.removeLines();
	
	ASSERT_INFO_ALWAYS(dom->children.size() == 2, "dom->children.size() = " << dom->children.size())
	ASSERT_ALWAYS(dynamic_cast<svgdom::PathElement*>(dom->children.begin()->get()))
	ASSERT_ALWAYS(dynamic_cast<svgdom::GElement*>((++dom->children.begin())->get()))
	ASSERT_ALWAYS(dynamic_cast<svgdom::GElement*>((++dom->children.begin())->get())->children.size() == 0)
}
