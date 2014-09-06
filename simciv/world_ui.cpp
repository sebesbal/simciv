#include "world_ui.h"
#include "AppMacros.h"
#include "VisibleRect.h"
#include "ui\UICheckBox.h"
#include "ui\UILayout.h"
#include "ui\UIHBox.h"
#include "ui\UIVBox.h"
#include "ui\UIText.h"

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
	CheckBox* chb = CheckBox::create("cocosui/check_box_normal.png",
										"cocosui/check_box_normal_press.png",
										"cocosui/check_box_active.png",
										"cocosui/check_box_normal_disable.png",
										"cocosui/check_box_active_disable.png");
	chb->setSelectedState(true);
	chb->addEventListener(cb);
	l->addChild(chb);
	
    auto label = Text::create();
	label->setString(text);
	l->addChild(label);

	return l;
}


void WorldUI::init_menu()
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto w = visibleSize.width;
	auto h = visibleSize.height;

	Vec2 topLeft = Vec2(visibleSize.width / 2.0f, visibleSize.height / 2.0f);

	// right menu
	double scale = 0.6;
	double space = 7;

	auto factory_button = MenuItemImage::create("factory1.png", "factory1.png", CC_CALLBACK_1(WorldUI::set_factory_mode, this) );
	factory_button->setScale(0.4 * scale);
	//factory_button->setPosition(Vec2(visibleSize) - factory_button->getScale() * Vec2(factory_button->getContentSize() / 2));

	auto mine_button = MenuItemImage::create("mine2.png", "mine2.png", CC_CALLBACK_1(WorldUI::set_mine_mode, this) );
	mine_button->setScale(0.25 * scale);
	auto b = factory_button->getBoundingBox();
	//mine_button->setPosition(Vec2(b.getMaxX(), b.getMinY() - space) - mine_button->getScale() * Vec2(mine_button->getContentSize() / 2));

    // create menu, it's an autorelease object
	auto menu = Menu::create(factory_button, mine_button, NULL);
	menu->setPosition(0, h - 100);
    this->addChild(menu, 1);
	menu->alignItemsVertically();


	// left menu
	auto cb_bck = labelled_cb("Background", [this](Ref* pSender,CheckBox::EventType type) {
		_map->setVisible(type == CheckBox::EventType::SELECTED);
	});
	cb_bck->setPosition(Vec2(100, h - 100));

	this->addChild(cb_bck);
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
	_model.create_map(10, 10, 1);
	_mode = IT_FACTORY;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
    //auto label = LabelTTF::create("Hello World", "Arial", TITLE_FONT_SIZE);
    //
    //// position the label on the center of the screen
    //label->setPosition(Vec2(origin.x + visibleSize.width/2,
    //                        origin.y + visibleSize.height - label->getContentSize().height));

    //// add the label as a child to this layer
    //this->addChild(label, 1);

	    // add "HelloWorld" splash screen"
    _map = Sprite::create("map.png");

    // position the sprite on the center of the screen
    _map->setPosition(Vec2(visibleSize / 2) + origin);
	_map->setScale(1);
    // add the sprite as a child to this layer
    this->addChild(_map, 0, 0);
	_map->setLocalZOrder(-1);
    
    //auto listener = EventListenerTouchAllAtOnce::create();
	auto listener = EventListenerTouchOneByOne::create();
	listener->onTouchMoved = CC_CALLBACK_2(WorldUI::onTouchMoved, this);
	listener->onTouchBegan = CC_CALLBACK_2(WorldUI::onTouchBegan, this);
	listener->onTouchEnded = CC_CALLBACK_2(WorldUI::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	//Node*
	_items = Node::create(); 
	_items->setPosition(Vec2(visibleSize / 2) + origin);
	
	_map->addChild(_items, 0, 1);
	_items->setLocalZOrder(1);

	//auto factory1 = Sprite::create("factory1.png");
	//factory1->setPosition(10, 10);
	//factory1->setScale(0.2);
	//_items->addChild(factory1);

	//auto mine1 = Sprite::create("mine2.png");
	//mine1->setPosition(100, 10);
	//mine1->setScale(0.15);
	//_items->addChild(mine1);

	//auto coal1 = Sprite::create("mine2.png");
	//coal1->setPosition(mine1->getPosition());
	//coal1->setScale(0.05);
	//_items->addChild(coal1);
	//auto actionTo = MoveTo::create(1.5, factory1->getPosition());
	//auto moveBack = MoveTo::create(0, mine1->getPosition());
	//auto seq = Sequence::create(actionTo, moveBack, NULL);
	//coal1->runAction(RepeatForever::create(seq));
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
	// DrawPrimitives::drawLine( Vec2(b.getMinX(), b.getMinY()), Vec2(b.getMaxX(), b.getMaxY()));

	float x = b.getMinX();
	for (int i = 0; i <= _model.width(); ++i, x += cs)
	{
		DrawPrimitives::drawLine( Vec2(x, b.getMinY()), Vec2(x, b.getMaxY()));
	}
}

Rect WorldUI::get_rect(int x, int y)
{
	auto b = _map->getBoundingBox();
	return Rect(cs * x, cs * y, cs, cs);
}

Item* WorldUI::add_item(ItemType type, int x, int y)
{
	int cell_size = 50;
	int ax = x / cell_size;
	int ay = y / cell_size;
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

void WorldUI::set_factory_mode(Ref* sender)
{
	_mode = IT_FACTORY;
}

void WorldUI::set_mine_mode(Ref* sender)
{
	_mode = IT_MINE;
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
	auto diff2 = diff; //diff / 5;
    auto back = _map; // getChildByTag(0);
    auto currentPos = back->getPosition();
    back->setPosition(currentPos + diff2);
	//auto stars = getChildByTag(1);
	//currentPos = stars->getPosition();
	//stars->setPosition(currentPos + diff);
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