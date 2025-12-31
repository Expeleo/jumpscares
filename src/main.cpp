#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

/* =========================
   CAMERA SHAKE ACTION
   ========================= */
class CameraShake : public CCActionInterval {
    CCPoint m_startPos;
    float m_strength;

public:
    static CameraShake* create(float duration, float strength) {
        auto ret = new CameraShake();
        if (ret && ret->initWithDuration(duration)) {
            ret->m_strength = strength;
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    void startWithTarget(CCNode* target) override {
        CCActionInterval::startWithTarget(target);
        m_startPos = target->getPosition();
    }

    void update(float) override {
        float x = CCRANDOM_MINUS1_1() * m_strength;
        float y = CCRANDOM_MINUS1_1() * m_strength;
        m_target->setPosition(m_startPos + ccp(x, y));
    }

    void stop() override {
        m_target->setPosition(m_startPos);
        CCActionInterval::stop();
    }
};

/* =========================
   JUMPSCARE MANAGER
   ========================= */
class JumpscareManager : public CCNode {
    float m_timer = 0.f;

public:
    static JumpscareManager* create() {
        auto ret = new JumpscareManager();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init() {
        this->scheduleUpdate();
        return true;
    }

    void update(float dt) override {
        m_timer += dt;

        if (m_timer >= 1.0f) {
            m_timer = 0.f;

            // 1 / 1000 chance
            if (rand() % 1000 == 0) {
                doJumpscare();
            }
        }
    }

    void doJumpscare() {
        auto director = CCDirector::sharedDirector();
        auto scene = director->getRunningScene();
        auto winSize = director->getWinSize();

        if (!scene) return;

        /* FLASH */
        auto flash = CCLayerColor::create(
            ccc4(255, 255, 255, 255),
            winSize.width,
            winSize.height
        );
        flash->setZOrder(10000);
        flash->runAction(CCSequence::create(
            CCFadeOut::create(0.15f),
            CCCallFuncN::create(flash, callfuncN_selector(CCNode::removeFromParent)),
            nullptr
        ));
        scene->addChild(flash);

        /* SHAKE */
        scene->runAction(CameraShake::create(0.25f, 18.f));

        /* ANIMATION */
        Vector<CCSpriteFrame*> frames;
        for (int i = 1; i <= 10; i++) {
            frames.pushBack(
                CCSpriteFrame::create(
                    fmt::format("jump_{}.png", i).c_str(),
                    CCRect(0, 0, 512, 512)
                )
            );
        }

        auto animation = CCAnimation::createWithSpriteFrames(frames, 0.03f);
        auto sprite = CCSprite::createWithSpriteFrame(frames.front());
        sprite->setPosition(winSize / 2);
        sprite->setScale(2.3f);
        sprite->setZOrder(9999);

        sprite->runAction(CCSequence::create(
            CCAnimate::create(animation),
            CCCallFuncN::create(sprite, callfuncN_selector(CCNode::removeFromParent)),
            nullptr
        ));
        scene->addChild(sprite);

        /* SOUND */
        CocosDenshion::SimpleAudioEngine::sharedEngine()
            ->playEffect("jumpscare.wav", false);
    }
};

/* =========================
   PLAYLAYER HOOK
   ========================= */
class $modify(MyPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;

        auto manager = JumpscareManager::create();
        this->addChild(manager, 9999);

        return true;
    }
};
