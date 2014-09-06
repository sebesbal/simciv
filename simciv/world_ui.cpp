#include "world_ui.h"
#include "AppMacros.h"
#include "VisibleRect.h"
#include "ui\UICheckBox.h"
#include "ui\UILayout.h"
#include "ui\UIHBox.h"
#include "ui\UIVBox.h"
#include "ui\UIText.h"
#include "ui\UIButton.h"
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

Layout* labelled_cb(std::string text, CheckBox::ccCheckBoxCallback cb)
{
	auto l = HBox::create();
	auto p = LinearLayoutParameter::create();
	p->setGravity(LinearLayoutParameter::LinearGravity::CENTER_VERTICAL);

	CheckBox* chb = CheckBox::create("cocosui/check_box_normal.png",
										"cocosui/check_box_normal_press.png",
										"cocosui/check_box_active.png",
										"cocosui/check_box_normal_disable.png",
										"cocosui/check_box_active_disable.png");
	chb->setSelectedState(true);
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

	auto mine_button = Button::create("mine2.png", "mine2.png", "mine2.png");
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
	auto cb_bck = labelled_cb("Background", [this](Ref* pSender,CheckBox::EventType type) {
		_map->setVisible(type == CheckBox::EventType::SELECTED);
	});
	cb_bck->setLayoutParameter(p);
	auto cb_grid = labelled_cb("Grid", [this](Ref* pSender,CheckBox::EventType type) {
		_show_grid = !_show_grid;
	});
	cb_grid->setLayoutParameter(p);

	left_menu->addChild(cb_bck);
	left_menu->addChild(cb_grid);

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
	_show_grid = true;

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
    return true;
}

void WorldUI::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
    _customCommand.init(_globalZOrder);
    _customCommand.func = CC_CALLBACK_0(WorldUI::onDraw, this, transform, flags);
    renderer->addCommand(&_customCommand);
}

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
}

Rect WorldUI::get_rect(int x, int y)
{
	auto b = _map->getBoundingBox();
	return Rect(cs * x, cs * y, cs, cs);
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
			auto mine1 = Sprite::create("mine2.png");
			mine1->setPosition(x, y);
			mine1->setScale(0.15);
			_items->addChild(mine1);
			_model.add_supply(a, 0, 50, 10);
		}
		break;
	case simciv::IT_FACTORY:
		{
			auto factory1 = Sprite::create("factory1.png");
			factory1->setPosition(x, y);
			factory1->setScale(0.2);
			_items->addChild(factory1);
			_model.add_supply(a, 0, -50, 100);
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