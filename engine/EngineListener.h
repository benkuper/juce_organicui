class EngineListener {
public:
	virtual ~EngineListener() {};

	virtual void startLoadFile() {};
	// TODO implement progression
	virtual void fileProgress(float /*percent*/, int /*state*/) {};
	virtual void endLoadFile() {};
};