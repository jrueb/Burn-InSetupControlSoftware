#ifndef HWDESCRIPTIONPARSER_H
#define HWDESCRIPTIONPARSER_H

#include <string>
#include <QString>
#include <QXmlStreamReader>

struct InstrumentDescription {
  std::string type;
  std::map<std::string, std::string> attrs;
  std::vector<std::map<std::string, std::string>> settings;
};

class HWDescriptionParser
{
public:
    HWDescriptionParser();
    std::vector<InstrumentDescription> ParseXML(QString pFileName);
private:
    InstrumentDescription ParseGeneric(const QXmlStreamReader *pXmlFile) const;
    
    InstrumentDescription ParseVoltageSource(QXmlStreamReader *pXmlFile, bool isLow);
    
    InstrumentDescription ParseChiller(QXmlStreamReader *pXmlFile);
    InstrumentDescription ParsePeltier(QXmlStreamReader *pXmlFile);
    InstrumentDescription ParseRaspberry(QXmlStreamReader *pXmlFile);
    
    InstrumentDescription ParseDAQModule(QXmlStreamReader *pXmlFile);
};

#endif // HWDESCRIPTIONPARSER_H
