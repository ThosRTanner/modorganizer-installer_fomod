/*
Copyright (C) 2012 Sebastian Herbord. All rights reserved.

This file is part of Mod Organizer.

Mod Organizer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Mod Organizer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Mod Organizer.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FOMODINSTALLERDIALOG_H
#define FOMODINSTALLERDIALOG_H

#include "directorytree.h"
#include "guessedvalue.h"
#include "ipluginlist.h"

#include <QDialog>
#include <QGroupBox>
#include <QMetaType>
#include <QObject>
#include <QString>

#include <functional>
#include <vector>

class QAbstractButton;
class QXmlStreamReader;

namespace Ui {
class FomodInstallerDialog;
}

namespace MOBase {
 class IOrganizer;
}

class ValueCondition;
class ConditionFlag;
class SubCondition;
class FileCondition;
class VersionCondition;

class XmlReader;

class IConditionTester {
public:
  virtual bool testCondition(int maxIndex, const ValueCondition *condition) const = 0;
  virtual bool testCondition(int maxIndex, const ConditionFlag *condition) const = 0;
  virtual bool testCondition(int maxIndex, const SubCondition *condition) const = 0;
  virtual bool testCondition(int maxIndex, const FileCondition *condition) const = 0;
  virtual bool testCondition(int maxIndex, const VersionCondition *condition) const = 0;
};


enum ConditionOperator {
  OP_AND,
  OP_OR
};

class Condition {
public:
  Condition() { }
  virtual bool test(int maxIndex, const IConditionTester *tester) const = 0;
private:
  Condition &operator=(const Condition&) = delete;
};

class ConditionFlag : public Condition {
public:
  ConditionFlag() : Condition(), m_Name(), m_Value() {}
  ConditionFlag(const QString &name, const QString &value) : Condition(), m_Name(name), m_Value(value) { }
  virtual bool test(int maxIndex, const IConditionTester *tester) const { return tester->testCondition(maxIndex, this); }
  QString m_Name;
  QString m_Value;
};
Q_DECLARE_METATYPE(ConditionFlag)

class ValueCondition : public Condition {
public:
  ValueCondition() : Condition(), m_Name(), m_Value() {}
  ValueCondition(const QString &name, const QString &value) : Condition(), m_Name(name), m_Value(value) { }
  virtual bool test(int maxIndex, const IConditionTester *tester) const { return tester->testCondition(maxIndex, this); }
  QString m_Name;
  QString m_Value;
};
Q_DECLARE_METATYPE(ValueCondition)

class FileCondition : public Condition {
public:
  FileCondition() : Condition(), m_File(), m_State() {}
  FileCondition(const QString &file, const QString &state) : Condition(), m_File(file), m_State(state) {}
  virtual bool test(int maxIndex, const IConditionTester *tester) const { return tester->testCondition(maxIndex, this); }
  QString m_File;
  QString m_State;
};
Q_DECLARE_METATYPE(FileCondition)

class SubCondition : public Condition {
public:
  virtual bool test(int maxIndex, const IConditionTester *tester) const { return tester->testCondition(maxIndex, this); }
  ConditionOperator m_Operator;
  std::vector<Condition*> m_Conditions;
};
Q_DECLARE_METATYPE(SubCondition)

class VersionCondition : public Condition {
public:
  enum Type { v_Game, v_FOMM, v_FOSE };
  VersionCondition() : Condition(), m_Type(), m_RequiredVersion() {}
  VersionCondition(Type type, const QString &requiredVersion) : Condition(), m_Type(type), m_RequiredVersion(requiredVersion) { }
  virtual bool test(int maxIndex, const IConditionTester *tester) const { return tester->testCondition(maxIndex, this); }
  Type m_Type;
  QString m_RequiredVersion;
};
Q_DECLARE_METATYPE(VersionCondition)


class FileDescriptor : public QObject {
  Q_OBJECT
public:
  FileDescriptor(QObject *parent)
    : QObject(parent), m_Source(), m_Destination(), m_Priority(0), m_IsFolder(false), m_AlwaysInstall(false),
      m_InstallIfUsable(false),
      m_FileSystemItemSequence(0)
  {}

  FileDescriptor(const FileDescriptor &reference)
    : QObject(reference.parent()), m_Source(reference.m_Source), m_Destination(reference.m_Destination),
      m_Priority(reference.m_Priority), m_IsFolder(reference.m_IsFolder), m_AlwaysInstall(reference.m_AlwaysInstall),
      m_InstallIfUsable(reference.m_InstallIfUsable),
      m_FileSystemItemSequence(reference.m_FileSystemItemSequence)
  {}

  QString m_Source;
  QString m_Destination;
  int m_Priority;
  bool m_IsFolder;
  bool m_AlwaysInstall;
  bool m_InstallIfUsable;
  int m_FileSystemItemSequence;
private:
  FileDescriptor &operator=(const FileDescriptor&);
};

Q_DECLARE_METATYPE(FileDescriptor*)


class FomodInstallerDialog : public QDialog, public IConditionTester
{
  Q_OBJECT

public:
  explicit FomodInstallerDialog(const MOBase::GuessedValue<QString> &modName,
                                const QString &fomodPath,
                                const std::function<MOBase::IPluginList::PluginStates (const QString &)> &fileCheck,
                                QWidget *parent = 0);
  ~FomodInstallerDialog();

  void initData(MOBase::IOrganizer *moInfo);

  /**
   * @return bool true if the user requested the manual dialog
   **/
  bool manualRequested() const { return m_Manual; }

  /**
   * @return the (user-modified) name to be used for the mod
   **/
  QString getName() const;

  /**
   * @return the version of the mod as specified in the fomod info.xml
   */
  QString getVersion() const;

  /**
   * @return the mod id as specified in the info.xml
   */
  int getModID() const;

  /**
   * @return the mod url as specified in the fomod file
   */
  QString getURL() const;

  /**
   * @brief retrieve the updated archive tree from the dialog. The caller is responsible to delete the returned tree.
   *
   * @note This call is destructive on the input tree!
   *
   * @param tree input tree. (TODO isn't this the same as the tree passed in the constructor?)
   * @return DataTree* a new tree with only the selected options and directories arranged correctly. The caller takes custody of this pointer!
   **/
  MOBase::DirectoryTree *updateTree(MOBase::DirectoryTree *tree);

  bool hasOptions();

protected:

  virtual bool eventFilter(QObject *object, QEvent *event);

private slots:

  void on_cancelBtn_clicked();

  void on_manualBtn_clicked();

  void on_websiteLabel_linkActivated(const QString &link);

  void on_nextBtn_clicked();

  void on_prevBtn_clicked();

  //detect signals for people playing with checkboxes/buttons
  void widgetButtonClicked();

private:

  enum ItemOrder {
    ORDER_ASCENDING,
    ORDER_DESCENDING,
    ORDER_EXPLICIT
  };

//So I can make GroupType and PluginTypeInfo into QVariants
public:
  enum GroupType {
    TYPE_SELECTATLEASTONE,
    TYPE_SELECTATMOSTONE,
    TYPE_SELECTEXACTLYONE,
    TYPE_SELECTANY,
    TYPE_SELECTALL
  };

  enum PluginType {
    TYPE_REQUIRED,
    TYPE_RECOMMENDED,
    TYPE_OPTIONAL,
    TYPE_NOTUSABLE,
    TYPE_COULDBEUSABLE
  };

  struct DependencyPattern {
    PluginType type;
    SubCondition condition;
  };

  typedef std::vector<DependencyPattern> DependencyPatternList;

  struct PluginTypeInfo
  {
    PluginType m_DefaultType;
    DependencyPatternList m_DependencyPatterns;
  };

private:
  typedef std::vector<FileDescriptor*> FileDescriptorList;
  typedef std::vector<ConditionFlag> ConditionFlagList;

  struct Plugin {
    QString m_Name;
    QString m_Description;
    QString m_ImagePath;
    PluginTypeInfo m_PluginTypeInfo;
    ConditionFlagList m_ConditionFlags;
    FileDescriptorList m_Files;
  };

  struct ConditionalInstall {
    SubCondition m_Condition;
    FileDescriptorList m_Files;
  };

  struct LeafInfo {
    int priority;
    QString path;
  };

  typedef std::map<int, LeafInfo> Leaves;

private:

  QString readContent(QXmlStreamReader &reader);
  void readInfoXml();
  void readModuleConfigXml();
  void parseInfo(QXmlStreamReader &data);

  void updateNameEdit();

  static int bomOffset(const QByteArray &buffer);
  static ItemOrder getItemOrder(const QString &orderString);
  static GroupType getGroupType(const QString &typeString);
  static PluginType getPluginType(const QString &typeString);
  static bool byPriority(const FileDescriptor *LHS, const FileDescriptor *RHS);

  PluginType getPluginDependencyType(int page, PluginTypeInfo const &info) const;

  bool copyFileIterator(MOBase::DirectoryTree *sourceTree, MOBase::DirectoryTree *destinationTree,
                        const FileDescriptor *descriptor,
                        Leaves *leaves, MOBase::DirectoryTree::Overwrites *overwrites);

  typedef void (FomodInstallerDialog::*TagProcessor)(XmlReader &reader);
  void processXmlTag(XmlReader &reader, char const *tag, TagProcessor func);

  void readFileList(XmlReader &reader, FileDescriptorList &fileList);
  void readDependencyPattern(XmlReader &reader, DependencyPattern &pattern);
  void readDependencyPatternList(XmlReader &reader, DependencyPatternList &patterns);
  void readDependencyPluginType(XmlReader &reader, PluginTypeInfo &info);
  void readPluginType(XmlReader &reader, Plugin &plugin);
  void readConditionFlagList(XmlReader &reader, ConditionFlagList &condflags);
  FomodInstallerDialog::Plugin readPlugin(XmlReader &reader);
  void readPluginList(XmlReader &reader, QString const &groupName, GroupType &groupType, QLayout *layout);
  void readGroup(XmlReader &reader, QLayout *layout);
  void readGroupList(XmlReader &reader, QLayout *layout);
  QGroupBox *readInstallStep(XmlReader &reader);
  void readCompositeDependency(XmlReader &reader, SubCondition &conditional);
  ConditionalInstall readConditionalInstallPattern(XmlReader &reader);
  void readConditionalFilePatternList(XmlReader &reader);
  void readConditionalFileInstallList(XmlReader &reader);
  void readStepList(XmlReader &reader);
  void readModuleConfiguration(XmlReader &reader);
  void parseModuleConfig(XmlReader &data);
  void highlightControl(QAbstractButton *button);

  bool testCondition(int maxIndex, const QString &flag, const QString &value) const;
  virtual bool testCondition(int maxIndex, const ValueCondition *condition) const;
  virtual bool testCondition(int maxIndex, const ConditionFlag *condition) const;
  virtual bool testCondition(int maxIndex, const SubCondition *condition) const;
  virtual bool testCondition(int maxIndex, const FileCondition *condition) const;
  virtual bool testCondition(int maxIndex, const VersionCondition *condition) const;
  bool testVisible(int pageIndex) const;
  bool nextPage();
  void activateCurrentPage();
  void moveTree(MOBase::DirectoryTree::Node *target, MOBase::DirectoryTree::Node *source, MOBase::DirectoryTree::Overwrites *overwrites);
  MOBase::DirectoryTree::Node *findNode(MOBase::DirectoryTree::Node *node, const QString &path, bool create);
  void copyLeaf(MOBase::DirectoryTree::Node *sourceTree, const QString &sourcePath,
                MOBase::DirectoryTree::Node *destinationTree, const QString &destinationPath,
                MOBase::DirectoryTree::Overwrites *overwrites, Leaves *leaves, int pri);

  static void FomodInstallerDialog::applyPriority(Leaves *leaves, MOBase::DirectoryTree::Node *node, int priority);

  static QString toString(MOBase::IPluginList::PluginStates state);

  //Set the 'next' button to display 'next' or 'install'
  void updateNextbtnText();

  //Display the current page calculating all the button enables/disables
  void displayCurrentPage();

private:

  Ui::FomodInstallerDialog *ui;

  MOBase::GuessedValue<QString> m_ModName;

  int m_ModID;

  QString m_FomodPath;
  bool m_Manual;

  FileDescriptorList m_RequiredFiles;
  std::vector<ConditionalInstall> m_ConditionalInstalls;
  std::vector<bool> m_PageVisible;

  std::function<MOBase::IPluginList::PluginStates (const QString&)> m_FileCheck;

  //Because NMM maintains the sequence from the xml when dealing with things with
  //the same priority, we have to as well. This is moderately hacky.
  int m_FileSystemItemSequence;

  //So I can find out game info (I hope)
  MOBase::IOrganizer *m_MoInfo;

  //The web page in the fomod (if supplied)
  QString m_URL;

};

Q_DECLARE_METATYPE(FomodInstallerDialog::GroupType)
Q_DECLARE_METATYPE(FomodInstallerDialog::PluginTypeInfo)

#endif // FOMODINSTALLERDIALOG_H
