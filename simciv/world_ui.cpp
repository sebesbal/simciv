#include "world_ui.h"
#include "AppMacros.h"
#include "VisibleRect.h"
#include "ui\UICheckBox.h"
#include "ui\UILayout.h"
#include "ui\UIHBox.h"
#include "ui\UIVBox.h"
#include "ui\UIText.h"
#include "ui\UIButton.h"
//#include "CC
#include "base\ccTypes.h"
#include <algorithm>
#include "economy.h"

namespace simciv
{

USING_NS_CC;
using namespace std;

std::string factory_strings[4] = {
	"factory_red.png", "factory_blue.png", "factory_green.png", "factory_yellow.png"
};

std::string mine_strings[4] = {
	"mine_red.png", "mine_blue.png", "mine_green.png", "mine_yellow.png"
};

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




Layout* combobox(const std::string* labels)
{
	return NULL;
}

VBox* left_menu;


#define RBON "cocosui/Radio_button_on.png"
#define RBOFF "cocosui/Radio_button_off.png"

class RadioBox: public HBox
{
public:
	RadioBox (int* data, std::vector<std::string> labels, int hh, int marginy): data(data), hh(hh), marginy(marginy)
	{
		int k = 0;
		for (auto l: labels)
		{
			auto cb = [this, k](Ref* pSender, CheckBox::EventType type) { setSelected(k); };
			Widget* rb;
			if (l.substr(0, 1) == "_")
			{
				rb = image_radio(l.substr(1, l.length() - 1), cb);
			}
			else
			{
				rb = labelled_radio(l, cb);
			}
			++k;
			addChild(rb);
			items.push_back(rb);
		}
		setSelected(0);
	}
	void setSelected(int i)
	{
		int l = 0;
		for (Widget* item: items)
		{
			((CheckBox*)item->getChildren().at(0))->setSelectedState(i == l++);
		}
		*(this->data) = i;
	}

	static RadioBox* create(int* data, std::vector<std::string> labels, int hh, int marginy)
	{
	    RadioBox* widget = new RadioBox(data, labels, hh, marginy);
		if (widget && widget->init())
		{
			widget->autorelease();
			widget->setSize(Size(100, hh + marginy));
			return widget;
		}
		else
		{
			CC_SAFE_DELETE(widget);
			return nullptr;
		}
	}
	Layout* labelled_radio(std::string text, CheckBox::ccCheckBoxCallback cb)
	{
		auto l = HBox::create();
		auto p = LinearLayoutParameter::create();
		p->setGravity(LinearLayoutParameter::LinearGravity::CENTER_VERTICAL);

		CheckBox* chb = CheckBox::create(RBOFF, RBOFF, RBON, RBOFF, RBON);
		chb->setSelectedState(false);
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

		LinearLayoutParameter* pp = LinearLayoutParameter::create();
		pp->setGravity(LinearLayoutParameter::LinearGravity::TOP);
		pp->setMargin(Margin(2, marginy, 2, 2));

		l->setLayoutParameter(pp);

		return l;
	}
	Layout* image_radio(string img, CheckBox::ccCheckBoxCallback cb)
	{
		auto l = HBox::create();
		auto p = LinearLayoutParameter::create();
		p->setGravity(LinearLayoutParameter::LinearGravity::CENTER_VERTICAL);

		CheckBox* chb = CheckBox::create(RBOFF, RBOFF, RBON, RBOFF, RBON);
		chb->setSelectedState(false);
		chb->addEventListener(cb);
		chb->setLayoutParameter(p);
		l->addChild(chb);
	
		auto image = Widget::create();
		auto s = Sprite::create(img);
		s->setScale(hh / s->getContentSize().height);
		s->setPosition(hh/2, hh/2);
		image->addChild(s);
		image->setLayoutParameter(p);
		image->setSize(Size(hh, hh));
		l->addChild(image);
		//l->requestDoLayout();
		// auto height = std::max(chb->getSize().height, label->getSize().height);
		l->setSize(Size(100, hh));

		LinearLayoutParameter* pp = LinearLayoutParameter::create();
		pp->setGravity(LinearLayoutParameter::LinearGravity::TOP);
		pp->setMargin(Margin(2, marginy, 2, 2));

		l->setLayoutParameter(pp);

		return l;
	}
	int hh;
	int marginy;
	vector<Widget*> items;
	int* data;
};

#define defvec(vec, ...) \
	static const string arr ## vec[] = { __VA_ARGS__ }; \
	vector<string> vec (arr ## vec, arr ## vec + sizeof(arr ## vec) / sizeof(arr ## vec[0]) );

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
	p->setGravity(LinearLayoutParameter::LinearGravity::TOP);
	p->setMargin(Margin(2, 2, 2, 2));
	LinearLayoutParameter* q = LinearLayoutParameter::create();
	q->setGravity(LinearLayoutParameter::LinearGravity::LEFT);
	//q->setMargin(Margin(2, 2, 2, 2));

	for (int i = 0; i < 1; ++i)
	{
		auto hbox = HBox::create();
		hbox->setLayoutParameter(q);

		auto fs = factory_strings[i];
		auto factory_button = Button::create(fs, fs, fs);
		factory_button->addTouchEventListener([this, i](Ref* w, Widget::TouchEventType e) {
			// _product_id = i;
			_mode = IT_FACTORY;
		});
		
		factory_button->ignoreContentAdaptWithSize(false);
		factory_button->setSize(s);
		factory_button->setLayoutParameter(p);
		hbox->addChild(factory_button);

		fs = mine_strings[i];
		auto mine_button = Button::create(fs, fs, fs);
		mine_button->addTouchEventListener([this, i](Ref* w, Widget::TouchEventType e) {
			// _product_id = i;
			_mode = IT_MINE;
		});
		mine_button->setSize(s);
		mine_button->setLayoutParameter(p);
		mine_button->ignoreContentAdaptWithSize(false);
		hbox->addChild(mine_button);
		hbox->setSize(Size(50, 25));
		right_menu->addChild(hbox);
	}

	this->addChild(right_menu);
	//right_menu->setAnchorPoint(Vec2(1, 1)); // doesn't have effect
	//right_menu->setPosition(Vec2(w - 13, h));
	right_menu->setPosition(Vec2(w - 50, h));

	// left menu
	// auto 
	left_menu = VBox::create();
	p = LinearLayoutParameter::create();
	p->setMargin(Margin(2, 2, 2, 2));
	p->setGravity(LinearLayoutParameter::LinearGravity::LEFT);

	int hh = 30;
	int marginy = 20;

	// ==============================================================================================
	// PRODUCT
	defvec(vec0, "_factory_red.png", "_factory_blue.png", "_factory_green.png", "_factory_yellow.png")
	auto rb = RadioBox::create(&_product_id, vec0, hh, marginy);
	left_menu->addChild(rb);

	// ==============================================================================================
	// PRICE - VOL - RES
	_show_price_vol_mode = 0;
	defvec(vec1, "Price", "Volume", "Res.")
	rb = RadioBox::create(&_show_price_vol_mode, vec1, hh, marginy);
	left_menu->addChild(rb);

	// ==============================================================================================
	// SUPPLY - CONSUMPTION
	_show_sup_con_mode = 2;
	defvec(vec2, "Supply", "Cons.", "Both")
	rb = RadioBox::create(&_show_sup_con_mode, vec2, hh, marginy);
	left_menu->addChild(rb);

	// ==============================================================================================
	// BACKGROUND
	auto cb_bck = labelled_cb("Background", false, [this](Ref* pSender,CheckBox::EventType type) {
		_map->setVisible(type == CheckBox::EventType::SELECTED);
	});
	cb_bck->setLayoutParameter(p);
	left_menu->addChild(cb_bck);

	// ==============================================================================================--
	// TRANSPORT
	_show_transport = true;
	auto cb_transport = labelled_cb("Routes", _show_transport, [this](Ref* pSender,CheckBox::EventType type) {
		_show_transport = !_show_transport;
	});
	cb_transport->setLayoutParameter(p);
	left_menu->addChild(cb_transport);

	left_menu->setAnchorPoint(Vec2(0, 1));
	left_menu->setPosition(Vec2(0, h));

	this->addChild(left_menu);

	//set_price_vol_mode(0);
	//set_sup_con_mode(2);
	_show_grid = false;
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
	_map->setVisible(false);
	_model.create_map(_table.width / cs, _table.height / cs, 4);
    
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

	_product_id = 0;

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
	double min_vol = 1000;
	double max_vol = 0;

	if (_show_price_vol_mode == 0)
	{
		if (_show_sup_con_mode == 2)
		{
			for (Area* a: _model.areas())
			{
				auto& p = _model.get_prod(a, _product_id);
				double v = p.p;
				min_v = std::min(min_v, v);
				max_v = std::max(max_v, v);
				double vol = p.v_con + p.v_sup;
				min_vol = std::min(min_vol, vol);
				max_vol = std::max(max_vol, vol);
			}
			double d = max_v - min_v;
			double d_vol = max_vol - min_vol;

			for (Area* a: _model.areas())
			{
				auto& p = _model.get_prod(a, _product_id);
				double v = p.p;
				double r = d == 0 ? 0.5 : (v - min_v) / d;
				double vol = p.v_con + p.v_sup;
				draw_rect(a->x, a->y, r, vol / d_vol);
			}
		}
		else if (_show_sup_con_mode == 0)
		{
			for (Area* a: _model.areas())
			{
				auto& p = _model.get_prod(a, _product_id);
				double v = p.p_sup;
				min_v = std::min(min_v, v);
				max_v = std::max(max_v, v);
				double vol = p.v_sup;
				min_vol = std::min(min_vol, vol);
				max_vol = std::max(max_vol, vol);
			}
			double d = max_v - min_v;
			double d_vol = max_vol - min_vol;

			for (Area* a: _model.areas())
			{
				auto& p = _model.get_prod(a, _product_id);
				double v = p.p_sup;
				double r = d == 0 ? 0.5 : (v - min_v) / d;
				double vol = p.v_sup;
				draw_rect(a->x, a->y, r, vol / d_vol);
			}
		}
		else if (_show_sup_con_mode == 1)
		{
			for (Area* a: _model.areas())
			{
				auto& p = _model.get_prod(a, _product_id);
				double v = p.p_con;
				min_v = std::min(min_v, v);
				max_v = std::max(max_v, v);
				double vol = p.v_con;
				min_vol = std::min(min_vol, vol);
				max_vol = std::max(max_vol, vol);
			}
			double d = max_v - min_v;
			double d_vol = max_vol - min_vol;

			for (Area* a: _model.areas())
			{
				auto& p = _model.get_prod(a, _product_id);
				double v = p.p_con;
				double r = d == 0 ? 0.5 : (v - min_v) / d;
				double vol = p.v_con;
				draw_rect(a->x, a->y, r, vol / d_vol);
			}
		}
	}
	else if (_show_price_vol_mode == 2)
	{
		for (Area* a: _model.areas())
		{
			auto& p = _model.get_prod(a, _product_id);
			draw_rect_green(a->x, a->y, p.resource, 1);
		}
	}

	if (_show_transport)
	{
		//DrawPrimitives::setDrawColor4F(0, 0, 1, 1);
		glLineWidth(3);
		double scale = 0.1;

		for (Area* a: _model.areas())
		{
			double x, y;
			a->get_trans(_product_id, x, y);
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

void WorldUI::draw_rect_green(int x, int y, double rate, double alpha)
{
	Rect r = get_rect(x, y);
	DrawPrimitives::drawSolidRect( Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()), Color4F(0, rate, 0, alpha));
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
	_model.products()[_product_id]->routes_to_areas(_product_id);
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
			auto mine1 = Sprite::create(mine_strings[_product_id]);
			mine1->setPosition(x, y);
			mine1->setScale(0.05);
			_items->addChild(mine1);
			_model.add_supply(a, _product_id, 100, 10);
		}
		break;
	case simciv::IT_FACTORY:
		{
			auto factory1 = Sprite::create(factory_strings[_product_id]);
			factory1->setPosition(x, y);
			factory1->setScale(0.2);
			_items->addChild(factory1);
			_model.add_supply(a, _product_id, -100, 100);
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

void WorldUI::set_price_vol_mode(int i)
{
	int k = 0;
	for (Widget* l: _cb_price_vol_mode)
	{
		((CheckBox*)l->getChildren().at(0))->setSelectedState(i == k++);
	}
	_show_price_vol_mode = i;
}

void WorldUI::set_sup_con_mode(int i)
{
	int k = 0;
	for (Widget* l: _cb_sup_con_mode)
	{
		((CheckBox*)l->getChildren().at(0))->setSelectedState(i == k++);
	}
	_show_sup_con_mode = i;
}

}