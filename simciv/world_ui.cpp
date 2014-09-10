#include "world_ui.h"
#include "AppMacros.h"
#include "VisibleRect.h"
#include "ui\UICheckBox.h"
#include "ui\UILayout.h"
#include "ui\UIHBox.h"
#include "ui\UIVBox.h"
#include "ui\UIText.h"
#include "ui\UIButton.h"
#include "base\ccTypes.h"
#include <algorithm>

namespace simciv
{

USING_NS_CC;


Scene* WorldUI::scene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    WorldUI *layer = WorldUI::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

using namespace ui;

Layout* labelled_cb(std::string text, bool checked, CheckBox::ccCheckBoxCallback cb)
{
	auto l = HBox::create();
	auto p = LinearLayoutParameter::create();
	p->setGravity(LinearLayoutParameter::LinearGravity::CENTER_VERTICAL);

	CheckBox* chb = CheckBox::create("cocosui/check_box_normal.png",
										"cocosui/check_box_normal_press.png",
										"cocosui/check_box_active.png",
										"cocosui/check_box_normal_disable.png",
										"cocosui/check_box_active_disable.png");
	chb->setSelectedState(checked);
	chb->addEventListener(cb);
	chb->setLayoutParameter(p);
	l->addChild(chb);
	
    auto label = Text::create();
	label->setString(text);
	label->setFontSize(18);
	label->setLayoutParameter(p);
	l->addChild(label);
	l->requestDoLayout();
	auto height = std::max(chb->getSize().height, label->getSize().height);
	l->setSize(Size(100, height));

	return l;
}

VBox* left_menu;

void WorldUI::init_menu()
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto w = visibleSize.width;
	auto h = visibleSize.height;

	Vec2 topLeft = Vec2(visibleSize.width / 2.0f, visibleSize.height / 2.0f);

	// right menu
	auto right_menu = VBox::create();

	auto s = Size(20, 20);
	LinearLayoutParameter* p = LinearLayoutParameter::create();
	p->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);
	p->setMargin(Margin(2, 2, 2, 2));

	auto factory_button = Button::create("factory1.png", "factory1.png", "factory1.png");
	factory_button->addTouchEventListener([this](Ref* w, Widget::TouchEventType e) {
		_mode = IT_FACTORY;
	});
		
	factory_button->ignoreContentAdaptWithSize(false);
	factory_button->setSize(s);
	factory_button->setLayoutParameter(p);
	right_menu->addChild(factory_button);

	// auto mine_button = Button::create("mine2.png", "mine2.png", "mine2.png");
	auto mine_button = Button::create("mine3.png", "mine3.png", "mine3.png");
	mine_button->addTouchEventListener([this](Ref* w, Widget::TouchEventType e) {
		_mode = IT_MINE;
	});
	mine_button->setSize(s);
	mine_button->setLayoutParameter(p);
	mine_button->ignoreContentAdaptWithSize(false);
	right_menu->addChild(mine_button);
	this->addChild(right_menu);
	right_menu->setAnchorPoint(Vec2(1, 1)); // doesn't have effect
	right_menu->setPosition(Vec2(w - 13, h));


	// left menu
	// auto 
	left_menu = VBox::create();
	p = LinearLayoutParameter::create();
	p->setMargin(Margin(2, 2, 2, 2));
	p->setGravity(LinearLayoutParameter::LinearGravity::LEFT);

	auto cb_bck = labelled_cb("Background", true, [this](Ref* pSender,CheckBox::EventType type) {
		_map->setVisible(type == CheckBox::EventType::SELECTED);
	});
	cb_bck->setLayoutParameter(p);
	left_menu->addChild(cb_bck);
	
	_show_grid = false;
	//auto cb_grid = labelled_cb("Grid", _show_grid, [this](Ref* pSender,CheckBox::EventType type) {
	//	_show_grid = !_show_grid;
	//});
	//cb_grid->setLayoutParameter(p);
	//left_menu->addChild(cb_grid);

	_show_price = true;
	auto cb_price = labelled_cb("Price", _show_price, [this](Ref* pSender,CheckBox::EventType type) {
		_show_price = !_show_price;
	});
	cb_price->setLayoutParameter(p);
	left_menu->addChild(cb_price);

	_show_volume = true;
	auto cb_volume = labelled_cb("Volume", _show_volume, [this](Ref* pSender,CheckBox::EventType type) {
		_show_volume = !_show_volume;
	});
	cb_volume->setLayoutParameter(p);
	left_menu->addChild(cb_volume);

	_show_supply = false;
	auto cb_supply = labelled_cb("Supply", _show_supply, [this](Ref* pSender,CheckBox::EventType type) {
		_show_supply = !_show_supply;
	});
	cb_supply->setLayoutParameter(p);
	left_menu->addChild(cb_supply);

	_show_demand = false;
	auto cb_demand = labelled_cb("Demand", _show_demand, [this](Ref* pSender,CheckBox::EventType type) {
		_show_demand = !_show_demand;
	});
	cb_demand->setLayoutParameter(p);
	left_menu->addChild(cb_demand);

	_show_transport = true;
	auto cb_transport = labelled_cb("Flow", _show_transport, [this](Ref* pSender,CheckBox::EventType type) {
		_show_transport = !_show_transport;
	});
	cb_transport->setLayoutParameter(p);
	left_menu->addChild(cb_transport);

	left_menu->setAnchorPoint(Vec2(0, 1));
	left_menu->setPosition(Vec2(0, h));

	this->addChild(left_menu);
}

// on "init" you need to initialize your instance
bool WorldUI::init()
{
    if ( !Layer::init() )
    {
        return false;
    }
    
	init_menu();

	// init model
	
	_mode = IT_FACTORY;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    _map = Sprite::create("map.png");

	_table = _map->getContentSize();

    // position the sprite on the center of the screen
	//_map->setAnchorPoint(Vec2(0, 0));
    _map->setPosition(Vec2(visibleSize / 2));
	_map->setScale(1);
    // add the sprite as a child to this layer
    this->addChild(_map, 0, 0);
	_map->setLocalZOrder(-1);

	_model.create_map(_table.width / cs, _table.height / cs, 1);
    
    //auto listener = EventListenerTouchAllAtOnce::create();
	auto listener = EventListenerTouchOneByOne::create();
	listener->onTouchMoved = CC_CALLBACK_2(WorldUI::onTouchMoved, this);
	listener->onTouchBegan = CC_CALLBACK_2(WorldUI::onTouchBegan, this);
	listener->onTouchEnded = CC_CALLBACK_2(WorldUI::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	//Node*
	_items = Node::create(); 
	_items->setAnchorPoint(Vec2(0, 0));
	_items->setPosition(Vec2(visibleSize / 2) - _table / 2);
	//_items->setPosition(Vec2(0, 0));
	
	this->addChild(_items, 0, 1);
	_items->setLocalZOrder(1);
	_items->setContentSize(_table);

	
	//auto cb = [](float f) {};
	// _draw_tiles.schedule(schedule_selector(WorldUI::tick), this, 0.1, kRepeatForever, 0, false);
	this->schedule(schedule_selector(WorldUI::tick), 0.05, kRepeatForever, 0);

	add_item(IT_FACTORY, _table.width / 3, _table.height / 2);
	add_item(IT_MINE, 2 * _table.width / 3, _table.height / 2);

    return true;
}

void WorldUI::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
    _customCommand.init(_globalZOrder);
    _customCommand.func = CC_CALLBACK_0(WorldUI::onDraw, this, transform, flags);
    renderer->addCommand(&_customCommand);
}
int lofusz = 0;
void WorldUI::onDraw(const Mat4 &transform, uint32_t flags)
{
    Director* director = Director::getInstance();
    director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, transform);
    
    //draw
    CHECK_GL_ERROR_DEBUG();
    
    // draw a simple line
    // The default state is:
    // Line Width: 1
    // color: 255,255,255,255 (white, non-transparent)
    // Anti-Aliased
    //  glEnable(GL_LINE_SMOOTH);
    //DrawPrimitives::drawLine( VisibleRect::leftBottom(), VisibleRect::rightTop() );
	auto b = _map->getBoundingBox();
	//DrawPrimitives::drawLine( Vec2(b.getMinX(), b.getMinY()), Vec2(b.getMaxX(), b.getMaxY()));
	//DrawPrimitives::drawLine( Vec2(b.getMinX(), b.getMaxY()), Vec2(b.getMaxX(), b.getMinY()));

	if (_show_grid)
	{
		glLineWidth(1);
		float x = b.getMinX();
		for (int i = 0; i <= _model.width(); ++i, x += cs)
		{
			DrawPrimitives::drawLine( Vec2(x, b.getMinY()), Vec2(x, b.getMaxY()));
		}
		float y = b.getMinY();
		for (int i = 0; i <= _model.height(); ++i, y += cs)
		{
			DrawPrimitives::drawLine( Vec2(b.getMinX(), y), Vec2(b.getMaxX(), y));
		}
	}

	// draw_rect(5, 5, 1);

	double min_v = 1000;
	double max_v = 0;
	double max_vol = 0;

	if (_show_price)
	{
		for (Area* a: _model.areas())
		{
			double v = a->_prod[0].p;
			min_v = std::min(min_v, v);
			max_v = std::max(max_v, v);
			max_vol = std::max(max_v, a->_prod[0].v);
		}
		double d = max_v - min_v;

		for (Area* a: _model.areas())
		{
			double v = a->_prod[0].p;
			double r = d == 0 ? 0.5 : (v - min_v) / d;
			draw_rect(a->x, a->y, r, a->_prod[0].v / max_vol);
		}
	}

	if (_show_supply)
	{
		for (Area* a: _model.areas())
		{
			double v = a->_prod[0].v;
			min_v = std::min(min_v, v);
			max_v = std::max(max_v, v);
		}
		double d = max_v - min_v;

		for (Area* a: _model.areas())
		{
			double v = a->_prod[0].v;
			double r = d == 0 ? 0.5 : (v - min_v) / d;
			draw_rect(a->x, a->y, r, 0.5);
		}
	}

	if (_show_transport)
	{
		//DrawPrimitives::setDrawColor4F(0, 0, 1, 1);
		glLineWidth(3);
		double scale = 1;

		for (Area* a: _model.areas())
		{
			double x, y;
			a->get_trans(0, x, y);
			Rect r = get_rect(a->x, a->y);
			Vec2 p = Vec2(r.getMidX(), r.getMidY());
			DrawPrimitives::drawLine(p, Vec2(p.x + scale * x, p.y + scale * y));
		}
	}
}

void WorldUI::draw_rect(int x, int y, double rate, double alpha)
{
	Rect r = get_rect(x, y);
	
	DrawPrimitives::drawSolidRect( Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(1 - rate, rate, 0, alpha));
}


void WorldUI::draw_vec(Vec2 a, Vec2 v)
{

}

Rect WorldUI::get_rect(int x, int y)
{
	auto b = _map->getBoundingBox();
	return Rect(b.getMinX() + cs * x, b.getMinY() + cs * y, cs, cs);
}


void WorldUI::tick(float f)
{
	_model.end_turn();
}

Item* WorldUI::add_item(ItemType type, int x, int y)
{
	int ax = x / cs;
	int ay = y / cs;
	if (ax < 0 || ay < 0 || ax >= _model.width() || ay >= _model.height()) return NULL;

	Area* a = _model.get_area(ax, ay);

	switch (type)
	{
	case simciv::IT_MINE:
		{
			auto mine1 = Sprite::create("mine3.png");
			mine1->setPosition(x, y);
			mine1->setScale(0.05);
			_items->addChild(mine1);
			_model.add_supply(a, 0, 100, 10);
		}
		break;
	case simciv::IT_FACTORY:
		{
			auto factory1 = Sprite::create("factory1.png");
			factory1->setPosition(x, y);
			factory1->setScale(0.2);
			_items->addChild(factory1);
			_model.add_supply(a, 0, -100, 100);
		}
		break;
	default:
		break;
	}

	return NULL;
}

void WorldUI::onEnter()
{
	Layer::onEnter();
}

void WorldUI::menuCloseCallback(Ref* sender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

void WorldUI::onTouchMoved(Touch* touch, Event  *event)
{
    auto diff = touch->getDelta();
	_map->setPosition(_map->getPosition() + diff);
	_items->setPosition(_items->getPosition() + diff);
}

bool WorldUI::onTouchBegan(Touch* touch, Event  *event)
{
    return true;
}

void WorldUI::onTouchEnded(Touch* touch, Event  *event)
{
	auto s = touch->getStartLocation();
	auto p = touch->getLocation();
	if ((p - s).length() < 10)
	{
		p = _items->convertToNodeSpace(p);
		add_item(_mode, p.x, p.y);
	}
}

}