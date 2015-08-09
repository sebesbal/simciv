#include "world_ui.h"
#include "economy.h"

namespace simciv
{

USING_NS_CC;
using namespace std;
using namespace ui;


AnimalView* AnimalView::create(WorldModel* model)
{
	AnimalView* result = new AnimalView();
	result->_model = model;
	if (result && result->init())
	{
		result->autorelease();
		return result;
	}
	else
	{
		CC_SAFE_DELETE(result);
		return nullptr;
	}
}

bool AnimalView::init()
{
	if (!MapView::init())
	{
		return false;
	}

	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto w = visibleSize.width;
	auto h = visibleSize.height;

	left_menu = VBox::create();
	auto p = LinearLayoutParameter::create();
	p->setMargin(Margin(2, 2, 2, 2));
	p->setGravity(LinearLayoutParameter::LinearGravity::LEFT);
}

bool AnimalView::is_map_point(cocos2d::Vec2& p)
{
	return p.x > 370 || p.y > 222;
}

void AnimalView::onTouchEnded(Touch* touch, Event  *event)
{
	auto s = touch->getStartLocation();
	auto p = touch->getLocation();
	if (is_map_point(touch->getLocationInView()) && (p - s).length() < 10)
	{
		//p = _items->convertToNodeSpace(p);
		//add_item(_mode, p.x, p.y);
	}
}


void AnimalView::onTouchMoved(Touch* touch, Event  *event)
{
	if (is_map_point(touch->getLocationInView()))
	{
		auto diff = touch->getDelta();
		_map->setPosition(_map->getPosition() + diff);
		//_items->setPosition(_items->getPosition() + diff);
	}
}

void AnimalView::onDraw(const Mat4 &transform, uint32_t flags)
{
	// calculate roads
	//_model->products()[_product_id]->routes_to_areas(_product_id);

}

}