//
// Created by wgsdp on 31/03/2026.
//

#include "Scenes/SceneSerializer.h"
#include "FreeCamera.h"
#include "LightingSystem.h"
#include "Render3DSystem.h"

#include <iostream>
#include <cstring>

#include "SerialFile.h"
#include "SerialTree.h"

namespace GPC
{
    // =========================================================================
    //  Helpers de test
    // =========================================================================

    static int s_Passed = 0;
    static int s_Failed = 0;

#define SERIAL_TEST(name, condition)                                                    \
    do {                                                                                \
        if (condition) {                                                                \
            std::cout << "  [OK]   " << name << "\n";                                   \
            ++s_Passed;                                                                 \
        } else {                                                                        \
            std::cout << "  [FAIL] " << name << "\n";                                   \
            ++s_Failed;                                                                 \
        }                                                                               \
    } while(0)

#define SERIAL_SECTION(name) std::cout << "\n--- " << name << " ---\n"

    static void PrintResults()
    {
        std::cout << "\n========================================\n";
        std::cout << "  Passed : " << s_Passed << "\n";
        std::cout << "  Failed : " << s_Failed << "\n";
        std::cout << "========================================\n\n";
        s_Passed = 0;
        s_Failed = 0;
    }

    // =========================================================================
    //  TESTS SERIALTREE — structure mémoire
    // =========================================================================

    static void Test_SerialTree()
    {
        std::cout << "\n╔══════════════════════════════════════╗\n";
        std::cout <<   "║         TESTS SERIAL TREE            ║\n";
        std::cout <<   "╚══════════════════════════════════════╝\n";

        // --- Scalaires ---
        SERIAL_SECTION("Scalaires");
        {
            float f = 3.14f;
            SerialTree node;
            node.Set(&f);
            SERIAL_TEST("float — IsData",              node.IsData());
            SERIAL_TEST("float — ByteSize == 4",       node.ByteSize() == sizeof(float));
            SERIAL_TEST("float — Get correct",         *node.Get<float>() == 3.14f);
            SERIAL_TEST("float — Count == 1",          node.Count<float>() == 1);
        }
        {
            int i = -42;
            SerialTree node;
            node.Set(&i);
            SERIAL_TEST("int   — Get correct",         *node.Get<int>() == -42);
        }
        {
            double d = 1.23456789;
            SerialTree node;
            node.Set(&d);
            SERIAL_TEST("double — Get correct",        *node.Get<double>() == 1.23456789);
        }

        // --- Tableaux ---
        SERIAL_SECTION("Tableaux");
        {
            int arr[] = {10, 20, 30, 40};
            SerialTree node;
            node.Set(arr, 4);
            SERIAL_TEST("int[4] — Count == 4",         node.Count<int>() == 4);
            SERIAL_TEST("int[4] — Get[0] == 10",       node.Get<int>()[0] == 10);
            SERIAL_TEST("int[4] — Get[3] == 40",       node.Get<int>()[3] == 40);
            SERIAL_TEST("int[4] — ByteSize == 16",     node.ByteSize() == 4 * sizeof(int));
        }
        {
            float arr[] = {1.0f, 2.0f, 3.0f};
            SerialTree node;
            node.Set(arr, 3);
            SERIAL_TEST("float[3] — Get[1] == 2.0f",  node.Get<float>()[1] == 2.0f);
        }

        // --- Ownership ---
        SERIAL_SECTION("Ownership");
        {
            int val = 42;
            SerialTree node;
            node.Set(&val, 1, true); // copie
            val = 99;
            SERIAL_TEST("copy — Get toujours 42 après modif source", *node.Get<int>() == 42);
        }
        {
            int val = 42;
            SerialTree node;
            node.Set(&val, 1, false); // référence
            val = 99;
            SERIAL_TEST("ref  — Get reflète la modif (99)",          *node.Get<int>() == 99);
        }

        // --- Navigation et arbre ---
        SERIAL_SECTION("Navigation / arbre");
        {
            int   hp    = 100;
            int   dmg[] = {5, 10, 15};
            float spd   = 1.5f;

            SerialTree player;
            player["hp"].Set(&hp);
            player["weapon"]["dmg"].Set(dmg, 3);
            player["weapon"]["speed"].Set(&spd);

            SERIAL_TEST("player — IsMap",                 player.IsMap());
            SERIAL_TEST("player — ChildCount == 2",       player.ChildCount() == 2);
            SERIAL_TEST("player — HasKey(hp)",            player.HasKey("hp"));
            SERIAL_TEST("player — HasKey(weapon)",        player.HasKey("weapon"));
            SERIAL_TEST("hp     — IsData",                player["hp"].IsData());
            SERIAL_TEST("hp     — Get == 100",            *player["hp"].Get<int>() == 100);
            SERIAL_TEST("weapon — IsMap",                 player["weapon"].IsMap());
            SERIAL_TEST("weapon — ChildCount == 2",       player["weapon"].ChildCount() == 2);
            SERIAL_TEST("dmg    — Get[0] == 5",           player["weapon"]["dmg"].Get<int>()[0] == 5);
            SERIAL_TEST("dmg    — Get[2] == 15",          player["weapon"]["dmg"].Get<int>()[2] == 15);
            SERIAL_TEST("speed  — Get == 1.5f",           *player["weapon"]["speed"].Get<float>() == 1.5f);
        }

        // --- Set sur un nœud DATA (réassignation) ---
        SERIAL_SECTION("Réassignation DATA");
        {
            int a = 1, b = 2;
            SerialTree node;
            node.Set(&a);
            SERIAL_TEST("avant réassignation — Get == 1", *node.Get<int>() == 1);
            node.Set(&b);
            SERIAL_TEST("après réassignation — Get == 2", *node.Get<int>() == 2);
        }

        PrintResults();
    }

    // =========================================================================
    //  TESTS SERIALFILE — Save / Load / LoadNode / Erreurs
    // =========================================================================

    static void Test_SerialFile()
    {
        std::cout << "\n╔══════════════════════════════════════╗\n";
        std::cout <<   "║         TESTS SERIAL FILE            ║\n";
        std::cout <<   "╚══════════════════════════════════════╝\n";

        // ---------------------------------------------------------------
        SERIAL_SECTION("Bug confirmé — Save racine DATA");
        // C'est le bug rapporté : un float seul à la racine ne se sauvegarde pas.
        // Save() exige une MAP en racine — un nœud DATA retourne TreeNotMap.
        // La solution est toujours d'envelopper dans une map.
        {
            float f = 3.14f;
            SerialTree node;
            node.Set(&f); // racine DATA — interdit
            ErrorType err = SerialFile::Save("Resources/bad_root.gpc", node);
            SERIAL_TEST("Save nœud DATA à la racine → TreeNotMap",
                        err == ErrorType::TREE_NOT_MAP);

            // Bonne façon — envelopper dans une map
            SerialTree root;
            root["value"].Set(&f);
            err = SerialFile::Save("Resources/good_root.gpc", root);
            SERIAL_TEST("Save float enveloppé dans une map → Success",
                        err == ErrorType::SUCCESS);
        }

        // ---------------------------------------------------------------
        SERIAL_SECTION("Save / Load — types de base");
        {
            const std::string path = "Resources/test_types.gpc";

            float   f   = 3.14f;
            int     i   = -42;
            double  d   = 1.23456789;
            uint8_t u   = 255;

            {
                SerialTree root;
                root["float"].Set(&f);
                root["int"].Set(&i);
                root["double"].Set(&d);
                root["uint8"].Set(&u);
                ErrorType err = SerialFile::Save(path, root);
                SERIAL_TEST("Save types de base → Success", err == ErrorType::SUCCESS);
            }
            {
                SerialTree loaded;
                ErrorType err = SerialFile::Load(path, loaded);
                SERIAL_TEST("Load → Success",                    err == ErrorType::SUCCESS);
                SERIAL_TEST("float  — Get == 3.14f",             *loaded["float"].Get<float>()   == 3.14f);
                SERIAL_TEST("int    — Get == -42",               *loaded["int"].Get<int>()        == -42);
                SERIAL_TEST("double — Get == 1.23456789",        *loaded["double"].Get<double>()  == 1.23456789);
                SERIAL_TEST("uint8  — Get == 255",               *loaded["uint8"].Get<uint8_t>()  == 255);
            }
        }

        // ---------------------------------------------------------------
        SERIAL_SECTION("Save / Load — tableau");
        {
            const std::string path = "Resources/test_array.gpc";
            int dmg[] = {10, 20, 30, 40, 50};

            {
                SerialTree root;
                root["dmg"].Set(dmg, 5);
                SerialFile::Save(path, root);
            }
            {
                SerialTree loaded;
                ErrorType err = SerialFile::Load(path, loaded);
                SERIAL_TEST("Load tableau → Success",          err == ErrorType::SUCCESS);
                SERIAL_TEST("dmg — Count == 5",                loaded["dmg"].Count<int>() == 5);
                SERIAL_TEST("dmg[0] == 10",                    loaded["dmg"].Get<int>()[0] == 10);
                SERIAL_TEST("dmg[4] == 50",                    loaded["dmg"].Get<int>()[4] == 50);
            }
        }

        // ---------------------------------------------------------------
        SERIAL_SECTION("Save / Load — arbre profond");
        {
            const std::string path = "Resources/test_deep.gpc";

            int   hp       = 100;
            int   dmg[]    = {5, 10, 15};
            float spd      = 1.5f;
            int   armor    = 30;

            {
                SerialTree root;
                root["player"]["hp"].Set(&hp);
                root["player"]["stats"]["armor"].Set(&armor);
                root["player"]["weapon"]["dmg"].Set(dmg, 3);
                root["player"]["weapon"]["speed"].Set(&spd);
                ErrorType err = SerialFile::Save(path, root);
                SERIAL_TEST("Save arbre profond → Success", err == ErrorType::SUCCESS);
            }
            {
                SerialTree loaded;
                ErrorType err = SerialFile::Load(path, loaded);
                SERIAL_TEST("Load arbre profond → Success",       err == ErrorType::SUCCESS);
                SERIAL_TEST("hp == 100",                           *loaded["player"]["hp"].Get<int>() == 100);
                SERIAL_TEST("armor == 30",                         *loaded["player"]["stats"]["armor"].Get<int>() == 30);
                SERIAL_TEST("dmg[1] == 10",                        loaded["player"]["weapon"]["dmg"].Get<int>()[1] == 10);
                SERIAL_TEST("speed == 1.5f",                       *loaded["player"]["weapon"]["speed"].Get<float>() == 1.5f);
            }
        }

        // ---------------------------------------------------------------
        SERIAL_SECTION("Save / Load — clés dupliquées (string pool)");
        // Même clé utilisée dans plusieurs maps — doit être dédupliquée dans le pool
        {
            const std::string path = "Resources/test_strpool.gpc";
            float x1 = 1.0f, y1 = 2.0f;
            float x2 = 3.0f, y2 = 4.0f;

            {
                SerialTree root;
                root["pos1"]["x"].Set(&x1);
                root["pos1"]["y"].Set(&y1);
                root["pos2"]["x"].Set(&x2);
                root["pos2"]["y"].Set(&y2);
                SerialFile::Save(path, root);
            }
            {
                SerialTree loaded;
                ErrorType err = SerialFile::Load(path, loaded);
                SERIAL_TEST("Load clés dupliquées → Success",  err == ErrorType::SUCCESS);
                SERIAL_TEST("pos1.x == 1.0f",                  *loaded["pos1"]["x"].Get<float>() == 1.0f);
                SERIAL_TEST("pos2.x == 3.0f",                  *loaded["pos2"]["x"].Get<float>() == 3.0f);
                SERIAL_TEST("pos1.y == 2.0f",                  *loaded["pos1"]["y"].Get<float>() == 2.0f);
                SERIAL_TEST("pos2.y == 4.0f",                  *loaded["pos2"]["y"].Get<float>() == 4.0f);
            }
        }

        // ---------------------------------------------------------------
        SERIAL_SECTION("LoadNode — chargement partiel");
        {
            const std::string path = "Resources/test_loadnode.gpc";

            int   hp    = 200;
            int   dmg[] = {5, 15};
            float spd   = 2.0f;
            int   gold  = 999;

            {
                SerialTree root;
                root["player"]["hp"].Set(&hp);
                root["player"]["weapon"]["dmg"].Set(dmg, 2);
                root["player"]["weapon"]["speed"].Set(&spd);
                root["world"]["gold"].Set(&gold);
                SerialFile::Save(path, root);
            }

            // Charge uniquement "player"
            {
                SerialTree player;
                ErrorType err = SerialFile::LoadNode(path, player, {"player"});
                SERIAL_TEST("LoadNode player → Success",       err == ErrorType::SUCCESS);
                SERIAL_TEST("player — IsMap",                  player.IsMap());
                SERIAL_TEST("player — HasKey(hp)",             player.HasKey("hp"));
                SERIAL_TEST("player — HasKey(weapon)",         player.HasKey("weapon"));
                SERIAL_TEST("player — !HasKey(world)",         !player.HasKey("world")); // world non chargé
                SERIAL_TEST("hp == 200",                       *player["hp"].Get<int>() == 200);
            }

            // Charge uniquement "player/weapon"
            {
                SerialTree weapon;
                ErrorType err = SerialFile::LoadNode(path, weapon, {"player", "weapon"});
                SERIAL_TEST("LoadNode weapon → Success",       err == ErrorType::SUCCESS);
                SERIAL_TEST("weapon — HasKey(dmg)",            weapon.HasKey("dmg"));
                SERIAL_TEST("weapon — HasKey(speed)",          weapon.HasKey("speed"));
                SERIAL_TEST("dmg[0] == 5",                     weapon["dmg"].Get<int>()[0] == 5);
                SERIAL_TEST("dmg[1] == 15",                    weapon["dmg"].Get<int>()[1] == 15);
                SERIAL_TEST("speed == 2.0f",                   *weapon["speed"].Get<float>() == 2.0f);
            }
        }

        // ---------------------------------------------------------------
        SERIAL_SECTION("Codes d'erreur");
        {
            // Fichier inexistant
            {
                SerialTree tree;
                ErrorType err = SerialFile::Load("Resources/inexistant.gpc", tree);
                SERIAL_TEST("Load inexistant → FILE_NOT_FOUND",
                            err == ErrorType::FILE_NOT_FOUND);
            }
            // LoadNode clé invalide
            {
                SerialTree tree;
                ErrorType err = SerialFile::LoadNode("Resources/test_deep.gpc", tree, {"cle_inexistante"});
                SERIAL_TEST("LoadNode clé invalide → INVALID_KEY_PATH",
                            err == ErrorType::INVALID_KEY_PATH);
            }
            // LoadNode chemin partiel valide puis invalide
            {
                SerialTree tree;
                ErrorType err = SerialFile::LoadNode("Resources/test_deep.gpc", tree, {"player", "inexistant"});
                SERIAL_TEST("LoadNode chemin partiel invalide → INVALID_KEY_PATH",
                            err == ErrorType::INVALID_KEY_PATH);
            }
            // LoadNode sur un nœud DATA (pas une MAP)
            {
                SerialTree tree;
                ErrorType err = SerialFile::LoadNode("Resources/test_deep.gpc", tree, {"player", "hp"});
                SERIAL_TEST("LoadNode sur nœud DATA → INVALID_KEY_PATH",
                            err == ErrorType::INVALID_KEY_PATH);
            }
        }

        PrintResults();
    }

    ErrorType SceneSerializer::OnCreate(SceneInformation &info)
    {
        SceneDefault::OnCreate(info);
        auto cController    = AddBehavior<FreeCamera>(mp_MainCamera->GetEntityID());    // On donne un script qui fait bouger la camera

        cController->MovementSpeed = 10; // Si on change un paramètre exposé d'une behaviour il s'update en live

        auto player  = CreateEntityAs3D();

        auto pRenderer      = AddComponent<Render3DComponent>(player);
        pRenderer->pGeometry = GPC_MESH("Cube"); // Trois type possible "Cube/Sphere/Plane"
        pRenderer->SetTexture("1024x1024", "Debug");

        auto light0 = CreateEntityAs3D();
        auto lLight0 = AddComponent<LightingComponent>(light0);
        auto lTransform0 = GetComponent<Transform3D>(light0);
        lLight0->pLightData = GPC_POINT_LIGHT("PointWhite");
        lTransform0->LocalTransform.AddPosition(2, 3, 2);

        // ═══════════════════════════════════════════════
        // TESTS SERIALISATION
        // ═══════════════════════════════════════════════
        Test_SerialTree();
        Test_SerialFile();

        return ErrorType::SUCCESS;
    }

    void SceneSerializer::OnDestroy()
    {

    }
}
