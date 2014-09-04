#include "Test1.h"
#include "AppMacros.h"

USING_NS_CC;


Scene* Test1::scene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    Test1 *layer = Test1::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

cocos2d::Node* planets;
cocos2d::Node* map;

// on "init" you need to initialize your instance
bool Test1::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                        "CloseNormal.png",
                                        "CloseSelected.png",
                                        CC_CALLBACK_1(Test1::menuCloseCallback,this));
    
    closeItem->setPosition(origin + Vec2(visibleSize) - Vec2(closeItem->getContentSize() / 2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
    
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
    map = Sprite::create("map.png");

    // position the sprite on the center of the screen
    map->setPosition(Vec2(visibleSize / 2) + origin);
	map->setScale(1);

    // add the sprite as a child to this layer
    this->addChild(map, 0, 0);
    
    auto listener = EventListenerTouchAllAtOnce::create();
    listener->onTouchesMoved = CC_CALLBACK_2(Test1::onTouchesMoved, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	//Node*
	planets = Node::create(); 
	planets->setPosition(Vec2(visibleSize / 2) + origin);
	map->addChild(planets, 0, 1);

	auto factory1 = Sprite::create("factory1.png");
	factory1->setPosition(10, 10);
	factory1->setScale(0.2);
	planets->addChild(factory1);

	auto mine1 = Sprite::create("mine2.png");
	mine1->setPosition(100, 10);
	mine1->setScale(0.15);
	planets->addChild(mine1);

	auto coal1 = Sprite::create("mine2.png");
	coal1->setPosition(mine1->getPosition());
	coal1->setScale(0.05);
	planets->addChild(coal1);
	auto actionTo = MoveTo::create(1.5, factory1->getPosition());
	auto moveBack = MoveTo::create(0, mine1->getPosition());
	auto seq = Sequence::create(actionTo, moveBack, NULL);
	coal1->runAction(RepeatForever::create(seq));
    return true;
}

void Test1::onEnter()
{
	Layer::onEnter();
}

void Test1::menuCloseCallback(Ref* sender)
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

void Test1::onTouchesMoved(const std::vector<Touch*>& touches, Event  *event)
{
    auto touch = touches[0];

    auto diff = touch->getDelta();
	auto diff2 = diff; //diff / 5;
    auto back = map; // getChildByTag(0);
    auto currentPos = back->getPosition();
    back->setPosition(currentPos + diff2);

	//auto stars = getChildByTag(1);
	//currentPos = stars->getPosition();
	//stars->setPosition(currentPos + diff);
}