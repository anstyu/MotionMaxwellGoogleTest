#include "pch.h"
#include <filesystem>
#include "MotionMaxwellInterface.h"

#if defined _MSC_VER
#include <windows.h>
#else
#include <dlfcn.h>
#include <stdio.h>
#endif

namespace fs = std::filesystem;

namespace MotionMaxwellTest {

#ifdef _WIN64
  const fs::path gTestRoot = fs::canonical("../..");
#else
  const fs::path gTestRoot = fs::canonical(".");
#endif
  const fs::path gProjectFolder = gTestRoot / "TestProjects";
  const fs::path gResultFolder = gTestRoot / "TestResults";

  void PrepareTestFolder(std::string folder)
  {
    if (fs::exists(gResultFolder) == false)
    {
      fs::create_directory(gResultFolder);
    }

    fs::path subFolder = gResultFolder / folder;
    if (fs::exists(subFolder) == true)
    {
      fs::remove_all(subFolder);
    }

    fs::create_directory(subFolder);
  }

  std::string CopyTestFile(std::string fileName, std::string folder, bool clean = true)
  {
    if (clean == true)
    {
      PrepareTestFolder(folder);
    }

    fs::path subFolder = gResultFolder / folder;
    // source file in the test project folder
    fs::path srcFile = fs::path(gProjectFolder) / fileName;
    EXPECT_TRUE(fs::exists(srcFile));

    // destination for copying the project
    fs::path destFile = subFolder / fileName;
    fs::copy_file(srcFile, destFile);
    EXPECT_TRUE(fs::exists(destFile));

    return fs::canonical(destFile).string();
  }

  std::string VectorToString(std::vector<std::string> vec)
  {
    if (vec.empty() == true)
    {
      return "[]";
    }

    std::string ret = "['";
    size_t end = vec.size() - 1;
    for (size_t i = 0; i <= end; i++)
    {
      ret += vec[i];
      if (i != end)
      {
        ret += "','";
      }
    }
    ret += "']";
    return ret;
  }

  void AreStringVectorsEqual(std::vector<std::string> vec1, std::vector<std::string> vec2)
  {
    std::string str1 = VectorToString(vec1);
    std::string str2 = VectorToString(vec2);
    EXPECT_EQ(str1, str2);
  }

  class InterfaceTest : public ::testing::Test {

  protected:
    void SetUp() override {
#ifdef _MSC_VER 
      //hInstLib = LoadLibrary(TEXT("D:\\repo3\\AnsysDev\\build_output\\64Debug\\MotionMaxwellCoupling.dll"));
      SetDllDirectory(TEXT("D:/repo3/AnsysDev/build_output/64Debug/"));
      hInstLib = LoadLibrary(TEXT("MotionMaxwellCoupling.dll"));
#else
      hInstLib = dlopen("./lib/libMotionMaxwellCPython.so", RTLD_LAZY);
      if (hInstLib == nullptr) {
        fprintf(stderr, "%s\n", dlerror());
      }

#endif
      EXPECT_TRUE(hInstLib != nullptr);
      if (hInstLib == nullptr) {
        return;
      }

#ifdef _MSC_VER
      auto funcAddress = GetProcAddress(hInstLib, "GetMotionMaxwellFactoryInterface");
      if (funcAddress != nullptr)
      {
        pInterface = (IMotionMaxwellInterface*)(funcAddress)();
      }
#else
      IMotionMaxwellInterface* (*funcAddress)();
      funcAddress = (IMotionMaxwellInterface* (*)())dlsym(hInstLib, "GetMotionMaxwellFactoryInterface");
      
      if (funcAddress != nullptr)
      {
        pInterface = (*funcAddress)();
      }
#endif
      
      EXPECT_TRUE(pInterface != nullptr);
    }

    void TearDown() override {
      if (pInterface != nullptr)
      {
        pInterface->SaveProject();
        pInterface->CloseProject();
        pInterface->ReleaseApplication();
        pInterface = nullptr;
      }
      if (hInstLib != nullptr)
      {
#ifdef _MSC_VER
        FreeLibrary(hInstLib);
#else
        dlclose(hInstLib);
#endif
        hInstLib = nullptr;
      }
    }
#ifdef _MSC_VER
    HMODULE hInstLib = nullptr;
#else
    void *hInstLib = nullptr;
#endif 
    IMotionMaxwellInterface* pInterface = nullptr;
  };
  // Test Error Messages
  TEST_F(InterfaceTest, DLL_GetMessagesTest1)
  {
    std::string copiedProject = CopyTestFile("test1.aedt", "DLL_GetMessagesTest1");
    bool result = pInterface->OpenProject(copiedProject, "Maxwell3DDesign1");
    EXPECT_TRUE(result);

    std::vector<std::string> msgs1;
    result = pInterface->GetMessages(msgs1);
    EXPECT_TRUE(result);

    std::vector<std::string> msgs2;
    result = pInterface->GetMessages(msgs2);
    EXPECT_TRUE(result);

    IMotionMaxwellInterface::Group group{ { "Box1", 1.5, 2.5, 3.5, {{-1,0,0},{0,0,1},{0,1,0}} }, { "Box1" } };
    std::map<std::string, IMotionMaxwellInterface::Material> mat{ {"Box1", {"copper", 1, 0, 0}} };
    result = pInterface->InitMagneticObjects({ group }, mat);
    EXPECT_FALSE(result);

    std::vector<std::string> msgs;
    result = pInterface->GetMessages(msgs);
    EXPECT_TRUE(result);
  }

  // Test the GetMaxwellDesigns function
  TEST_F(InterfaceTest, DLL_GetDesignNamesTest1)
  {
    // copy the test project to the temp folder to run the test
    std::string copiedProject = CopyTestFile("test1.aedt", "DLL_GetDesignNamesTest1");

    // calling the function to be tested
    std::vector<std::string> ret;
    bool result = pInterface->GetMaxwellDesigns(copiedProject, ret);
    EXPECT_TRUE(result);

    AreStringVectorsEqual(ret, { "Maxwell3DDesign1", "Maxwell3DDesign2" });
  }

  // Test the OpenProject function
  TEST_F(InterfaceTest, DLL_OpenProjectTest1)
  {
    std::string copiedProject = CopyTestFile("test1.aedt", "DLL_OpenProjectTest1");
    bool bRet = pInterface->OpenProject(copiedProject, "Maxwell3DDesign1");
    EXPECT_TRUE(bRet);
    bRet = pInterface->OpenProject(copiedProject, "Maxwell3DDesign1");
    EXPECT_TRUE(bRet);
    bRet = pInterface->OpenProject(copiedProject, "Maxwell3DDesign2");
    EXPECT_TRUE(bRet);
  }

  // Test the OpenProject function
  TEST_F(InterfaceTest, DLL_OpenMotionSolverTxtTest1)
  {
    std::string copiedProject = CopyTestFile("test1.aedt", "DLL_OpenMotionSolverTxtTest1");
    std::string copiedTextFile = CopyTestFile("solverinfo.txt", "DLL_OpenMotionSolverTxtTest1", false);
    bool bRet = pInterface->OpenProject(copiedTextFile, "Maxwell3DDesign1");
    EXPECT_TRUE(bRet);
    bRet = pInterface->OpenProject(copiedTextFile, "Maxwell3DDesign1");
    EXPECT_TRUE(bRet);
    bRet = pInterface->OpenProject(copiedTextFile, "Maxwell3DDesign2");
    EXPECT_TRUE(bRet);
  }

  // Test the InitMagneticObjects function
  TEST_F(InterfaceTest, DLL_InitObjectTest1)
  {
    std::string copiedProject = CopyTestFile("cs_test1.aedt", "DLL_InitObjectTest1");
    bool bRet = pInterface->OpenProject(copiedProject, "Maxwell3DDesign1");
    EXPECT_TRUE(bRet);

    IMotionMaxwellInterface::Group group{ { "Box1", 1.5, 2.5, 3.5, {{-1,0,0},{0,0,1},{0,1,0}} }, { "Box1" } };
    std::map<std::string, IMotionMaxwellInterface::Material> mat{ {"Box1", {"copper", 1, 0, 0}} };
    bRet = pInterface->InitMagneticObjects({ group }, mat);
    EXPECT_TRUE(bRet);
  }

  // Test the InitMagneticObjects function
  TEST_F(InterfaceTest, DLL_InitObjectTest2)
  {
    std::string copiedProject = CopyTestFile("cs_test2.aedt", "DLL_InitObjectTest2");
    bool bRet = pInterface->OpenProject(copiedProject, "Maxwell3DDesign1");
    EXPECT_TRUE(bRet);

    IMotionMaxwellInterface::Group group1{ { "Group1", 0.00125, 0.0017, 0.00325 }, {"Box1"} };
    IMotionMaxwellInterface::Group group2{ { "Group2", -0.0008 ,-0.0028 ,0.00055 }, {"Box3"} };
    std::map<std::string, IMotionMaxwellInterface::Material> mat{ {"Box1", {"copper", 1, 0, 0}},
                                                                  {"Box3", {"titanium", 1, 0, 0}} };
    bRet = pInterface->InitMagneticObjects({ group1, group2 }, mat);
    (bRet);
  }

  // Test the InitMagneticObjects function
  TEST_F(InterfaceTest, DLL_InitObjectTest3)
  {
    std::string copiedProject = CopyTestFile("cs_test3.aedt", "DLL_InitObjectTest3");
    bool bRet = pInterface->OpenProject(copiedProject, "Maxwell3DDesign1");
    EXPECT_TRUE(bRet);

    IMotionMaxwellInterface::Group group1{ { "Magnet1", 0, 0.0341221, 0 }, { "Magnet1", "Container1"} };
    IMotionMaxwellInterface::Group group2{ { "Magnet2", 0, 0, 0 }, {"Magnet2", "Container2"} };
    std::map<std::string, IMotionMaxwellInterface::Material> mat{ { "Magnet1", {"NdFe30",1,0,0} },
                                                                  { "Magnet2", {"NdFe30",0,1,0}} };
    bRet = pInterface->InitMagneticObjects({ group1, group2 }, mat);
    EXPECT_TRUE(bRet);
  }

  TEST_F(InterfaceTest, DLL_InitObjectTest4)
  {
    std::string copiedGeometry = CopyTestFile("materialtest.sab", "DLL_InitObjectTest4");

    fs::path folder = fs::path(copiedGeometry).parent_path();
    std::string projectPath = (folder / "materialtest.aedt").string();

    bool bRet = pInterface->CreateProject(projectPath, copiedGeometry);
    EXPECT_TRUE(bRet);

    IMotionMaxwellInterface::Group pos1{ {"Group0", 0, 0, 0 }, { "Box1" } };
    bRet = pInterface->InitMagneticObjects({ pos1 });
    EXPECT_TRUE(bRet);
  }

  // Unit test for importing geometry
  TEST_F(InterfaceTest, DLL_ImportGeometryTest1)
  {
    std::string copiedProject = CopyTestFile("test1.aedt", "DLL_ImportGeometryTest1");
    std::string copiedGeometry = CopyTestFile("geometry.sab", "DLL_ImportGeometryTest1", false);

    bool bRet = pInterface->OpenProject(copiedProject, "Maxwell3DDesign1");
    EXPECT_TRUE(bRet);

    bRet = pInterface->InitGeometry(copiedGeometry);
    EXPECT_TRUE(bRet);
  }

  // Unit test for creating project and import geometry
  TEST_F(InterfaceTest, DLL_CreateProjectTest1)
  {
    std::string copiedGeometry = CopyTestFile("geometry.sab", "DLL_CreateProjectTest1");

    fs::path folder = fs::path(copiedGeometry).parent_path();
    std::string projectPath = (folder / "new_test.aedt").string();

    bool bRet = pInterface->CreateProject(projectPath, copiedGeometry);
    EXPECT_TRUE(bRet);
  }

  // Unit test for creating project and import geometry
  TEST_F(InterfaceTest, DLL_AnalyzeProjectTest1)
  {
    std::string copiedProject = CopyTestFile("analyze_test1.aedt", "DLL_AnalyzeProjectTest1");

    bool bRet = pInterface->OpenProject(copiedProject, "Maxwell3DDesign1");
    EXPECT_TRUE(bRet);

    IMotionMaxwellInterface::Transform pos1{ "Magnet1", 0, 0.0341221, 0 };
    IMotionMaxwellInterface::Transform pos2{ "Magnet2", 0, 0, 0 };

    int flag;
    std::vector<IMotionMaxwellInterface::Result> result;
    bRet = pInterface->Analyze(0.0, { pos1, pos2 }, result, flag);
    EXPECT_TRUE(bRet);

    pos1 = { "Magnet1", 0, 0.03, 0 };
    pos2 = { "Magnet2", 0, 0, 0 };

    bRet = pInterface->Analyze(0.1, { pos1, pos2 }, result, flag);
    EXPECT_TRUE(bRet);

    pos1 = { "Magnet1", 0, 0.02, 0 };
    pos2 = { "Magnet2", 0, 0, 0 };

    bRet = pInterface->Analyze(0.2, { pos1, pos2 }, result, flag);
    EXPECT_TRUE(bRet);
  }

  // Creating a Maxwell design, importing geometry, add setups, solve, and extract results
  TEST_F(InterfaceTest, DLL_SenarioTest1)
  {
    std::string copiedGeometry = CopyTestFile("geometry2.sab", "DLL_SenarioTest1");

    fs::path folder = fs::path(copiedGeometry).parent_path();
    std::string projectPath = (folder / "senario1.aedt").string();

    bool bRet = pInterface->CreateProject(projectPath, copiedGeometry);
    EXPECT_TRUE(bRet);

    IMotionMaxwellInterface::Group pos1{ {"Group1", 0, 0.0341221, 0 }, { "Magnet1", "Container1" } };
    IMotionMaxwellInterface::Group pos2{ {"Group2", 0, 0, 0 }, { "Magnet2", "Container2" } };
    std::map<std::string, IMotionMaxwellInterface::Material> mat = { {"Magnet1", {"NdFe30"}},
                                                                     {"Magnet2", {"NdFe30"}},
                                                                     {"Container1", {"vacuum"}},
                                                                     {"Container2", {"vacuum"}},
                                                                     {"Region", {"vacuum"}},
                                                                     {"OuterContainer", {"vacuum"}},
                                                                     {"Iron_Under", {"iron"}} };
    bRet = pInterface->InitMagneticObjects({ pos1, pos2 }, mat);
    EXPECT_TRUE(bRet);

    int flag;
    IMotionMaxwellInterface::Transform trans1 = { "Group1", 0, 0.04, 0, {{-1,0,0},{0,0,1},{0,1,0}} };
    IMotionMaxwellInterface::Transform trans2 = { "Group2", 0, 0.01, 0, {{1,0,0},{0,0,-1},{0,1,0}} };
    std::vector<IMotionMaxwellInterface::Result> result;
    bRet = pInterface->Analyze(0.0, { trans1, trans2 }, result, flag);
    EXPECT_TRUE(bRet);

    bRet = pInterface->Analyze(0.0, { trans1, trans2 }, result, flag);
    EXPECT_TRUE(bRet);
    /*
    IMotionMaxwellInterface::Result r1 = result[0];
    AreDoubleEqual(-0.51907921181657801, r1.Fx);
    AreDoubleEqual(0.35293999416235999, r1.Fy);
    AreDoubleEqual(0.043405398425554000, r1.Fz);
    AreDoubleEqual(1.3902374905542800, r1.Tx);
    AreDoubleEqual(0.64017448593109905, r1.Ty);
    AreDoubleEqual(16.676347449022199, r1.Tz);

    IMotionMaxwellInterface::Result r2 = result[1];
    AreDoubleEqual(-0.54920243412659697, r2.Fx);
    AreDoubleEqual(-0.55632903095005304, r2.Fy);
    AreDoubleEqual(0.61188625375330596, r2.Fz);
    AreDoubleEqual(-1.8657788882217999, r2.Tx);
    AreDoubleEqual(1.0344515583182500, r2.Ty);
    AreDoubleEqual(-0.96431309081540701, r2.Tz);*/
  }

  TEST_F(InterfaceTest, DLL_SenarioTest2)
  {
    std::string copiedGeometry = CopyTestFile("2cubes.sab", "DLL_SenarioTest2");

    fs::path folder = fs::path(copiedGeometry).parent_path();
    std::string projectPath = (folder / "senario2.aedt").string();

    bool bRet = pInterface->CreateProject(projectPath, copiedGeometry);
    EXPECT_TRUE(bRet);

    IMotionMaxwellInterface::Group pos0{ {"Group0", 0, -0.02, 0.01 }, { "Box1" } };
    IMotionMaxwellInterface::Group pos1{ {"Group1", 0, 0.02, 0.01 }, { "Box2" } };
    IMotionMaxwellInterface::Group pos2{ {"Group2", 0.01, -0.03, 0.04 }, { "Box3" } };
    std::map<std::string, IMotionMaxwellInterface::Material> mat = { {"Box1", {"NdFe30"}},
                                                                     {"Box2", {"NdFe30"}},
                                                                     {"Box3", {"vacuum"}} };
    bRet = pInterface->InitMagneticObjects({ pos0, pos1, pos2 }, mat);
    EXPECT_TRUE(bRet);

    int flag;
    std::vector<IMotionMaxwellInterface::Result> result;
    bRet = pInterface->Analyze(0.0, { pos0.Trans, pos1.Trans, pos2.Trans }, result, flag);
    EXPECT_TRUE(bRet);
  }
}
