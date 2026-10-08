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

// --- High Accuracy Macro Replay Format Struct ---
struct MacroInput {
    int frame;
    int button;      
    bool isPress;
    double playerX;  
    double playerY;
};

// 
class UltimateGDModMixManager {
public:
    static UltimateGDModMixManager* get() {
        static UltimateGDModMixManager instance;
        return &instance;
    }

    //
    bool noclip = false;
    bool instantRespawn = false;
    bool startPosSwitcher = true;

    // 
    bool showHitboxes = false;
    bool layoutMode = false;
    bool showTrajectory = false;
    bool clockFromAttemptPercent = true; 

    //
    bool infiniteScaleBypass = true;
    bool absoluteObjectLimitBypass = true;

    // 🤖 Automation & Macros (Echo / xdBot)
    bool frameStepperActive = false;
    bool clickBotAudio = false;
    int currentFrame = 0;
    std::vector<MacroInput> activeMacro;
    size_t playbackIndex = 0;

    // 📂 MULTI-FORMAT REPLAY PARSER ROUTING ENGINE
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
            input.frame = deltaFrame;
            input.isPress = (inputByte & 1) != 0;
            input.button = (inputByte & 2) != 0 ? 2 : 1;
            activeMacro.push_back(input);
        }
    }
};

// =========================================================================
// 🎮 GAMEPLAY INTERCEPTIONS (PlayLayer)
// =========================================================================
class $modify(MixPlayLayer, PlayLayer) {
    void updateProgressbar() {
        PlayLayer::updateProgressbar();
        if (UltimateGDModMixManager::get()->clockFromAttemptPercent) {
            // Precision data parsing loop hooks here
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

// =========================================================================
// 🎨 ENGINE INPUT & VISUAL LOOPS (GJBaseGameLayer)
// =========================================================================
class $modify(MixBaseGameLayer, GJBaseGameLayer) {
    void update(float dt) {
        GJBaseGameLayer::update(dt);
        if (UltimateGDModMixManager::get()->showTrajectory && this->m_player1) {
            // Trajectory rendering calculations hook here
        }
    }

    void handleButton(bool push, int button, bool player1) {
        GJBaseGameLayer::handleButton(push, button, player1);
        if (UltimateGDModMixManager::get()->clickBotAudio && push) {
            // Sound processing logic registers here
        }
    }
};

//
// 
// 
class $modify(MixEditorLayer, LevelEditorLayer) {
    bool init(GJGameLevel* level, bool p1) {
        if (!LevelEditorLayer::init(level, p1)) return false;
        if (!Mod::get()->getSettingValue<bool>("enable-ai-gen")) return true;

        std::string difficulty = Mod::get()->getSettingValue<std::string>("gen-difficulty");
        std::string theme = Mod::get()->getSettingValue<std::string>("gen-theme");

        generateGameplayLayout(difficulty);
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
        log::info("Humanized AI Generation engine active. Target: {}", difficulty);

        for (int i = 0; i < totalStructures; i++) {
            //
            int blockLength = 3 + (std::rand() % 4); 
            
            // Re-tune scaling limits strictly for ILL Mode parameters (Top 10 Main List)
            if (difficulty == "ILL") {
                blockLength = 2 + (std::rand() % 2);
            }

            for (int b = 0; b < blockLength; b++) {
                float blockX = currentX + (b * 30.0f);
                auto block = this->createObject(1, CCPoint(blockX, currentY), false);
                if (block) this->m_objects->addObject(block);

                //
                if (b == 0 || b == blockLength - 1) {
                    float fillY = currentY - 30.0f;
                    while (fillY >= 105.0f) {
                        auto pillarBlock = this->createObject(1, CCPoint(blockX, fillY), false);
                        if (pillarBlock) this->m_objects->addObject(pillarBlock);
                        fillY -= 30.0f;
                    }
                }
            }

            //
            if (difficulty == "ILL") {
                for (int b = 0; b < blockLength; b++) {
                    auto ceiling = this->createObject(1, CCPoint(currentX + (b * 30.0f), currentY + 75.0f), false);
                    if (ceiling) this->m_objects->addObject(ceiling);
                    
                    if (b % 2 == 0) {
                        auto ceilingSpike = this->createObject(8, CCPoint(currentX + (b * 30.0f), currentY + 45.0f), false);
                        if (ceilingSpike) {
                            ceilingSpike->setRotation(180.0f); 
                            ceilingSpike->setScale(0.80f); // Tight frame bounds adjustment
                            this->m_objects->addObject(ceilingSpike);
                        }
                    }
                }
            } else {
                auto spike1 = this->createObject(8, CCPoint(currentX + 30.0f, currentY + 30.0f), false);
                auto spike2 = this->createObject(8, CCPoint(currentX + (blockLength * 30.0f) - 60.0f, currentY + 30.0f), false);
                if (spike1) this->m_objects->addObject(spike1);
                if (spike2) this->m_objects->addObject(spike2);
            }

            /
            if (i % 2 == 0) {
                float transitionX = currentX + (blockLength * 30.0f) + 45.0f;
                auto orb = this->createObject(36, CCPoint(transitionX, currentY + 30.0f), false);
                if (orb) this->m_objects->addObject(orb);
            }

            // Gapping calculations based on speed metrics
            float horizontalGap = (difficulty == "ILL") ? (70.0f + (std::rand() % 30)) : (140.0f + (std::rand() % 60)); 
            currentX += (blockLength * 30.0f) + horizontalGap;

            float heightShift = (std::rand() % 2 == 0) ? 30.0f : -30.0f;
