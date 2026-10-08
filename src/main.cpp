#include <Geode/Bindings.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/HardStreak.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <matjson.hpp>

using namespace geode::prelude;

struct MacroInput {
    int frame;
    int button;      
    bool isPress;
    double playerX;  
    double playerY;
};

// =========================================================================
// 🌐 ULTIMATE GD MOD MIX GLOBAL MANAGER
// =========================================================================
class UltimateGDModMixManager {
public:
    static UltimateGDModMixManager* get() {
        static UltimateGDModMixManager instance;
        return &instance;
    }

    bool noclip = false;
    bool instantRespawn = false;
    bool startPosSwitcher = true;
    bool showHitboxes = false;
    bool layoutMode = false;
    bool showTrajectory = false;
    bool clockFromAttemptPercent = true; 
    bool infiniteScaleBypass = true;
    bool absoluteObjectLimitBypass = true;
    bool frameStepperActive = false;
    bool clickBotAudio = false;
    int currentFrame = 0;
    std::vector<MacroInput> activeMacro;
    size_t playbackIndex = 0;

    void routeAndLoadMacro(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) return;
        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        activeMacro.clear();
        playbackIndex = 0;

        if (ext == ".json" || ext == ".echo") {
            parseEchoFormat(path);
        } else if (ext == ".gdr" || ext == ".gdr2") {
            parseGdrFormat(path);
        }
    }

private:
    void parseEchoFormat(const std::filesystem::path& path) {
        std::ifstream file(path);
        if (!file.is_open()) return;
        try {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            auto json = matjson::parse(content);
            if (json.contains("inputs") && json["inputs"].is_array()) {
                for (const auto& item : json["inputs"].as_array()) {
                    MacroInput input{};
                    input.frame = item["frame"].as_int();
                    input.isPress = item.contains("down") ? item["down"].as_bool() : item["isPress"].as_bool();
                    input.button = item.contains("p2") && item["p2"].as_bool() ? 2 : 1;
                    activeMacro.push_back(input);
                }
            }
        } catch (...) {}
    }

    void parseGdrFormat(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) return;
        char magic; file.read(magic, 3);
        if (magic != 'G' || magic != 'D' || magic != 'R') return;
        uint8_t version; file.read(reinterpret_cast<char*>(&version), 1);
        while (file.peek() != EOF) {
            MacroInput input{};
            uint32_t deltaFrame = 0; uint8_t inputByte = 0;
            file.read(reinterpret_cast<char*>(&deltaFrame), sizeof(deltaFrame));
            file.read(reinterpret_cast<char*>(&inputByte), sizeof(inputByte));
            file.read(reinterpret_cast<char*>(&input.playerX), sizeof(double));
            file.read(reinterpret_cast<char*>(&input.playerY), sizeof(double));
            input.frame = static_cast<int>(deltaFrame);
            input.isPress = (inputByte & 1) != 0;
            input.button = (inputByte & 2) != 0 ? 2 : 1;
            activeMacro.push_back(input);
        }
    }
};

// =========================================================================
// 📱 ANDROID FLOATING OVERLAY MENU LAYER
// =========================================================================
class FloatingMenuLayer : public FLAlertLayer {
public:
    static FloatingMenuLayer* create() {
        auto ret = new FloatingMenuLayer();
        if (ret && ret->init(240, 320, "GJ_square01.png", "Ultimate Mix Menu")) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool setup() override {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto background = CCScale9Sprite::create("GJ_square01.png");
        background->setContentSize(CCSize(320, 240));
        background->setPosition(winSize / 2);
        this->addChild(background);

        auto title = CCLabelBMFont::create("Ultimate Mod Mix", "bigFont.fnt");
        title->setPosition(winSize.width / 2, winSize.height / 2 + 100);
        title->setScale(0.6f);
        this->addChild(title);

        auto menu = CCMenu::create();
        auto noclipLabel = CCLabelBMFont::create("Toggle Noclip", "goldFont.fnt");
        noclipLabel->setScale(0.5f);
        auto noclipBtn = CCMenuItemSpriteExtra::create(
            noclipLabel, nullptr, this, menu_selector(FloatingMenuLayer::onToggleNoclip)
        );
        menu->addChild(noclipBtn);

        auto closeLabel = CCLabelBMFont::create("Close", "bigFont.fnt");
        closeLabel->setScale(0.4f);
        auto closeBtn = CCMenuItemSpriteExtra::create(
            closeLabel, nullptr, this, menu_selector(FloatingMenuLayer::onClose)
        );
        closeBtn->setPosition(0, -80);
        menu->addChild(closeBtn);

        menu->setPosition(winSize / 2);
        this->addChild(menu);
        return true;
    }

    void onToggleNoclip(CCObject*) {
        auto manager = UltimateGDModMixManager::get();
        manager->noclip = !manager->noclip;
        FLAlertLayer::create("Mix Menu", manager->noclip ? "Noclip ENABLED!" : "Noclip DISABLED!", "OK")->show();
    }

    void onClose(CCObject*) {
        this->removeFromParentAndCleanup(true);
    }
};

class $modify(MixPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto floatMenu = CCMenu::create();
        auto btnSprite = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        btnSprite->setOpacity(150); 
        
        auto floatBtn = CCMenuItemSpriteExtra::create(
            btnSprite, nullptr, this, menu_selector(MixPlayLayer::onOpenFloatingMenu)
        );
        
        floatMenu->addChild(floatBtn);
        floatMenu->setPosition(winSize.width - 30, winSize.height - 30);
        this->addChild(floatMenu, 999); 
        return true;
    }

    void onOpenFloatingMenu(CCObject*) {
        if (auto menuLayer = FloatingMenuLayer::create()) {
            this->addChild(menuLayer, 1000);
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* obstacle) {
        if (UltimateGDModMixManager::get()->noclip) return; 
        PlayLayer::destroyPlayer(player, obstacle);
    }

    void update(float dt) {
        UltimateGDModMixManager* manager = UltimateGDModMixManager::get();
        if (!this->m_isPaused) {
            manager->currentFrame++;
            if (manager->frameStepperActive) return;
        }
        PlayLayer::update(dt);
    }
};

class $modify(MixBaseGameLayer, GJBaseGameLayer) {
    void update(float dt) {
        GJBaseGameLayer::update(dt);
    }
};

// =========================================================================
// 🏗️ AI AUTO-GENERATOR & CONTEXTUAL THEME PARSER (LevelEditorLayer)
// =========================================================================
class $modify(MixEditorLayer, LevelEditorLayer) {
    bool init(GJGameLevel* level, bool p1) {
        if (!LevelEditorLayer::init(level, p1)) return false;
        if (!Mod::get()->getSettingValue<bool>("enable-ai-gen")) return true;

        std::string difficulty = Mod::get()->getSettingValue<std::string>("gen-difficulty");
        std::string theme = Mod::get()->getSettingValue<std::string>("gen-theme");

        if (difficulty == "custom") {
            std::string levelTarget = Mod::get()->getSettingValue<std::string>("custom-level-id");
            log::info("Custom Layout Profile Detected: Parsing canvas blocks from target: {}", levelTarget);
        } else {
            generateGameplayLayout(difficulty);
        }

        applyThemeDecoration(theme);
        return true;
    }

    void updateObjectScale(GameObject* obj, float scale) {
        if (UltimateGDModMixManager::get()->infiniteScaleBypass) {
            obj->setScale(scale); 
            return;
        }
        LevelEditorLayer::updateObjectScale(obj, scale);
    }

private:
    void generateGameplayLayout(const std::string& difficulty) {
        int totalStructures = 60; float currentX = 300.0f; float currentY = 105.0f;

        for (int i = 0; i < totalStructures; i++) {
            int blockLength = 3 + (std::rand() % 4); 
            if (difficulty == "ILL") {
                blockLength = 2 + (std::rand() % 2);
            }

            for (int b = 0; b < blockLength; b++) {
                float blockX = currentX + (b * 30.0f);
                auto block = this->createObject(1, CCPoint(blockX, currentY), false);
                if (block) this->m_objects->addObject(block);

                if (b == 0 || b == blockLength - 1) {
                    float fillY = currentY - 30.0f;
                    while (fillY >= 105.0f) {
                        auto pillarBlock = this->createObject(1, CCPoint(blockX, fillY), false);
                        if (pillarBlock) this->m_objects->addObject(pillarBlock);
                        fillY -= 30.0f;
                    }
                }
            }

            if (difficulty == "ILL") {
                for (int b = 0; b < blockLength; b++) {
                    auto ceiling = this->createObject(1, CCPoint(currentX + (b * 30.0f), currentY + 75.0f), false);
