#pragma once

#include <string>
#include <vector>
#include <map>
#include <stdexcept>

// Interface file for Motion-Maxwell coupling

class LaunchDesktopExeption : public std::runtime_error
{
public:
  explicit LaunchDesktopExeption(char const* const _Message) noexcept
  : std::runtime_error(_Message)
  {
  }
};

class IMotionMaxwellInterface {

public:

  // Structure to store transf information for an object
  struct Transform {
    std::string Name;
    double X;
    double Y;
    double Z;
    double A[3][3] = { {1,0,0},{0,1,0 },{0,0,1} };
  };

  // Structure to hold the information for groups
  struct Group
  {
    Transform Trans;
    std::vector<std::string> Objects;
  };

  // Structure to store output result from Maxwell
  struct Result {
    double Fx;
    double Fy;
    double Fz;
    double Tx;
    double Ty;
    double Tz;
  };

  struct Material
  {
    std::string Name;
    double X = 1;
    double Y = 0;
    double Z = 0;
  };

  // Enum for the severity levels of messages in AEDT
  enum MessageSeverity
  {
    //Has info icon.
    kInfoMessage,
    //Has warning icon.
    kWarningMessage,
    //Has error icon.
    kErrorMessage,
    //Message shows up in an AfxMessageBox.
    kFatalMessage
  };

  // Get the messages of the opened project in AEDT
  virtual bool GetMessages(std::vector<std::string>& messages, const MessageSeverity severity = kFatalMessage) = 0;

  // Get the list of Maxwell design names in an AEDT project
  virtual bool GetMaxwellDesigns(const std::string& projectFilePath, std::vector<std::string>& designList) = 0;

  // Opens a existing Maxwell project.
  // Optionally, design name can be specified, if empty, the first Maxwell Design will be used
  virtual bool OpenProject(const std::string& projectFilePath, const std::string& designName = "") noexcept(false) = 0;

  // Create a Maxwell project
  // The project should not exist, and a project with default setting shall be created.
  // This function will use the new design with the default design name.
  virtual bool CreateProject(const std::string& projectFilePath, const std::string& geometryFilePath) noexcept(false) = 0;

  // Export geometry from Motion to Maxwell.
  // Motion should first export its geometry to a file and provide the path to this file.
  virtual bool InitGeometry(const std::string& geometryFilePath) = 0;

  // Specify which objects are the Magnets and provide their mass center.
  // variables in Maxwell will be created in this function
  // a relative CS will be created at the center of each object
  virtual bool InitMagneticObjects(const std::vector<Group>& groups,
                                   const std::map<std::string, Material>& materials= std::map<std::string, Material>()) = 0;

  // provide the positions of the objects as input, analyzed the Maxwell project.
  // then get the output force and torque data.
  // (Question)The relative system for force and torque might not be consistent with transf in the future?
  virtual bool Analyze(const double time,
                       const std::vector<Transform>& objPositions,
                       std::vector<Result>& output,
                       int& stflag) = 0;

  // Save the project
  virtual bool SaveProject() = 0;

  // Close the project, saving it first
  virtual bool CloseProject() = 0;

  // IMPORTANT: call either QuitApplicaction() or Release() when coupling has finished.

  // Close electronic desktop, saving the project first
  virtual bool QuitApplication() = 0;

  // Release the electronic desktop without closing it.
  virtual bool ReleaseApplication() = 0;
};
