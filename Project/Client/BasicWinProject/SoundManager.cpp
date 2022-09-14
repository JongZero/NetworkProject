#include "pch.h"
#include "SoundManager.h"

FMOD_SYSTEM* SoundManager::FmodSoundSystem;

SoundManager* SoundManager::m_Instance = nullptr;

SoundManager::SoundManager()
	:m_ChannelCount(0)
{

}

SoundManager::~SoundManager()
{
	// ���� ���ϵ� ��� ������
	for (auto it = m_SoundFileMap.begin(); it != m_SoundFileMap.end(); it++)
	{
		FMOD_Sound_Release(it->second);
	}

	m_Instance->DeleteInstance();
}

void SoundManager::Initialize(int channelCount)
{
	FMOD_System_Create(&FmodSoundSystem);
	FMOD_System_Init(FmodSoundSystem, 32, FMOD_INIT_NORMAL, NULL);

	m_ChannelCount = channelCount;

	// ä���� �������ش�.
	for (int i = 0; i < m_ChannelCount; i++)
	{
		CreateChannel();
	}
}

void SoundManager::CreateChannel()
{
	JChannel* _channel = new JChannel();
	m_ChannelVec.push_back(_channel);
}

void SoundManager::Release()
{
	FMOD_System_Close(FmodSoundSystem);
	FMOD_System_Release(FmodSoundSystem);

// 	// ���带 �����ش�.
// 	for (auto it = m_SoundFileMap.begin(); it != m_SoundFileMap.end(); it++)
// 	{
// 		FMOD_SOUND* _deleteObject = it->second;
// 
// 		FMOD_Sound_Release(_deleteObject);
// 		_deleteObject = nullptr;
// 	}
// 
// 	// ä���� �����Ѵ�.
// 	for (int i = 0; i < m_ChannelCount; i++)
// 	{
// 		delete m_ChannelVec[i];
// 		m_ChannelVec[i] = nullptr;
// 	}
// 
// 	DeleteInstance();
}

void SoundManager::LoadSoundFile(CString folderPath)
{
	CFileFind _file;

	BOOL _isOpen = _file.FindFile(folderPath + _T("\\*.*"));
	CString _fileFilter = ".WAV";

	CString _filePath, _fileExt, _fileName;

	while (_isOpen)
	{
		_isOpen = _file.FindNextFile();

		if (_file.IsDirectory() && !_file.IsDots())			// ���丮 �߽߰� 
		{
			_filePath = _file.GetFilePath();

			LoadSoundFile(_filePath);						// ���������� �˻��ϱ� ���� ���ȣ�� �߻�  
		}

		_fileName = _file.GetFileName();
		_filePath = _file.GetFilePath();
		_fileExt = _filePath.Mid(_filePath.ReverseFind('.'));		// Ȯ���ڸ� �����Ѵ�. 
		_fileExt.MakeUpper();										// �빮��ȭ�Ѵ�.

		if (!_file.IsDots() && _fileExt == _fileFilter)				// ���� Ž�� ���� ���ǿ����� �ش� StringList�� �߰�
		{
			// ������ �̸�
			_fileName = _file.GetFileName();
			
			// Ȯ���ڴ� ����
			_fileName.Delete(_fileName.GetLength() - 4, 4);
			wstring _fileNameStr = ResourceManager::CStrToWStr(_fileName);

			// ������ ���
			string _filePathStr = ResourceManager::CStrToStr(folderPath);
			string _fileNameStrStr;
			_fileNameStrStr.assign(_fileNameStr.begin(), _fileNameStr.end());
			_filePathStr += "/";
			_filePathStr += _fileNameStrStr;
			_filePathStr += ".WAV";

			// FMOD ���� ���� ����
			// BGM����(Loop�� ������) üũ
			wstring _fileAttr;
			wstring _IsBGM = L"BGM";
			_fileAttr = _fileNameStr.substr(0, 3);

			if (_file.IsDirectory()) continue;						// ������ ���� ���� ������ �ȵ�

			// ���� ������ �����ϰ� map�� �ִ´�.
			if (_fileAttr == _IsBGM)
				CreateSound(_filePathStr.c_str(), _fileNameStr.c_str(), SoundInfo::Loop);
			else
				CreateSound(_filePathStr.c_str(), _fileNameStr.c_str(), SoundInfo::None);
		}
	}
}

void SoundManager::CreateSound(PCSTR path, PCWSTR soundName, SoundInfo info)
{
	FMOD_SOUND* _newSound;

	if (info == SoundInfo::Loop)
	{
		FMOD_System_CreateSound(FmodSoundSystem, path, FMOD_LOOP_NORMAL, 0, &_newSound);
	}
	else
	{
		FMOD_System_CreateSound(FmodSoundSystem, path, FMOD_LOOP_OFF, 0, &_newSound);
	}

	if (_newSound != nullptr)
		m_SoundFileMap[soundName] = _newSound;
}

void SoundManager::Play(int channelNum, CString soundName)
{
	if (channelNum < m_ChannelVec.size())
		FMOD_System_PlaySound(FmodSoundSystem, m_SoundFileMap[soundName], NULL, false, &m_ChannelVec[channelNum]->FmodChannel);
}

void SoundManager::Pause(int channelNum, CString soundName)
{
	if (channelNum < m_ChannelVec.size())
		FMOD_Channel_SetPaused(m_ChannelVec[channelNum]->FmodChannel, true);
}

void SoundManager::Resume(int channelNum, CString soundName)
{
	if (channelNum < m_ChannelVec.size())
		FMOD_Channel_SetPaused(m_ChannelVec[channelNum]->FmodChannel, false);
}

void SoundManager::Stop(int channelNum, CString soundName)
{
	if (channelNum < m_ChannelVec.size())
		FMOD_Channel_Stop(m_ChannelVec[channelNum]->FmodChannel);
}

void SoundManager::VolumeUp(int channelNum)
{
	if (channelNum < m_ChannelVec.size())
	{
			m_ChannelVec[channelNum]->Volume = SOUND_DEFAULT;

		FMOD_Channel_SetVolume(m_ChannelVec[channelNum]->FmodChannel, m_ChannelVec[channelNum]->Volume);
	}
}

void SoundManager::VolumeDown(int channelNum)
{
	if (channelNum < m_ChannelVec.size())
	{
		if (m_ChannelVec[channelNum]->Volume > SOUND_MIN)
		{
			m_ChannelVec[channelNum]->Volume = 0.5f;
		}

		FMOD_Channel_SetVolume(m_ChannelVec[channelNum]->FmodChannel, m_ChannelVec[channelNum]->Volume);
	}
}

void SoundManager::Update()
{
	// ä���� �÷��� ���� ��츸 �ش� ä���� ������Ʈ���ش�.
	for (int i = 0; i < m_ChannelVec.size(); i++)
	{
		FMOD_Channel_IsPlaying(m_ChannelVec[i]->FmodChannel, &m_ChannelVec[i]->IsPlaying);

		if (m_ChannelVec[i]->IsPlaying)
		{
			FMOD_System_Update(FmodSoundSystem);
		}
	}
}