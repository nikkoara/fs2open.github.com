#pragma once

#include "mission/dialogs/AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

class FictionViewerDialogModel: public AbstractDialogModel {
 Q_OBJECT

 public:
	 struct MusicOptionElement {
		 std::string name;
		 int id = -1;

		 MusicOptionElement(const char* Name, int Id)
		 : name(Name), id(Id) {
		 }
	 };

	FictionViewerDialogModel(QObject* parent, EditorViewport* viewport);
	~FictionViewerDialogModel() override = default;
	bool apply() override;

	void reject() override;

	const std::string& getStoryFile() const { return _storyFile; }
	const std::string& getFontFile() const { return _fontFile; }
	const std::string& getVoiceFile() const { return _voiceFile; }
	int getFictionMusic() const { return _fictionMusic; }
	const std::vector<MusicOptionElement>& getMusicOptions() const { return _musicOptions; }

	void setStoryFile(const std::string& storyFile) { modify<std::string>(_storyFile, storyFile); }
	void setFontFile(const std::string& fontFile) { modify<std::string>(_fontFile, fontFile); }
	void setVoiceFile(const std::string& voiceFile) { modify<std::string>(_voiceFile, voiceFile); }
	// TODO input validation on passed in fictionMusic?
	void setFictionMusic(int fictionMusic);

	int getMaxStoryFileLength() const { return _maxStoryFileLength; }
	int getMaxFontFileLength() const { return _maxFontFileLength; }
	int getMaxVoiceFileLength() const { return _maxVoiceFileLength; }

	bool query_modified() const;

	bool hasMultipleStages() const;
 private:
	void initializeData();

	template<typename T>
	void modify(T &a, const T &b);

	std::string _storyFile;
	std::string _fontFile;
	std::string _voiceFile;
	int		_fictionMusic;
	std::vector<MusicOptionElement> _musicOptions;

	int _maxStoryFileLength, _maxFontFileLength, _maxVoiceFileLength;
};


template<typename T>
inline void FictionViewerDialogModel::modify(T &a, const T &b) {
	if (a != b) {
		a = b;
		modelChanged();
	}
}

}
}
}
