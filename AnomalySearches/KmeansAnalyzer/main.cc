#include <KmeansAnalyzer.h>
#include <iostream>
#include <iterator>
#include <algorithm>

#include <vector>

#include "TROOT.h"
#include "TFile.h"
#include "TKey.h"
#include "TObject.h"
#include "TDirectory.h"
#include "TH2F.h"

namespace PLTU
{
    int const LASTROW = 79;
    int const LASTCOL = 51;
}

std::vector<std::string> tokenize(std::string line, char delim = ' ')
{
    std::stringstream ss(line);
    std::vector<std::string> tokens;
    std::string buf;
    while (getline(ss, buf, delim))
        tokens.push_back(buf);
    return tokens;
}

std::map<std::pair<unsigned, unsigned>, std::vector<int>> readMaskFile(std::string maskFilePath)
{

    std::map<std::pair<unsigned, unsigned>, std::vector<int>> _kmeansMasks;

    // naming from: https://github.com/cmsplt/PLTOffline/blob/c54f84ca9fd686568fdce01a5317e58bb24adbd0/RecenterMask.C#L38
    // according to the file the mask includes its edges and represents the inactive area
    const char *chanNamesC[25] = {"", "8 1 5", "8 1 13", "", // 0-3
                                  "8 1 21", "8 1 29", "",    // 4-6
                                  "8 2 5", "8 2 13", "",     // 7-9
                                  "8 2 21", "8 2 29", "",    // 10-12
                                  "7 1 5", "7 1 13", "",     // 13-15
                                  "7 1 21", "7 1 29", "",    // 16-18
                                  "7 2 5", "7 2 13", "",     // 19-21
                                  "7 2 21", "7 2 29", ""};   // 22-24
    std::vector<std::string> chanNames(chanNamesC, std::end(chanNamesC));

    std::ifstream maskFile;
    maskFile.open(maskFilePath);

    if (!maskFile.is_open())
    {
        std::cout << "EventAnalyzer::ReadMaskFile: Could not open file:" << maskFilePath << std::endl;
        // exit(1);
        return _kmeansMasks;
    }
    std::string line;
    while (getline(maskFile, line))
    {

        if (line.size() < 8)
            continue;

        if (line[0] == '#')
            continue;

        std::vector<std::string> tokens = tokenize(line);
        std::string chanString = tokens[0] + " " + tokens[1] + " " + tokens[2];

        int chanName_i = 0;
        while (chanName_i < chanNames.size() && chanNames[chanName_i].compare(chanString))
            ++chanName_i;

        if (chanName_i == chanNames.size())
        {
            std::cout << "EventAnalyzer::ReadMaskFile: Could not find mask for channel with signature:" << chanString << std::endl;
            // exit(1);
        }

        int channel = chanName_i;
        int ROC = std::stoi(tokens[3]);
        std::pair<int, int> chroc = std::make_pair(channel, ROC);

        std::vector<std::string> coltokens = tokenize(tokens[4], '-');
        std::vector<std::string> rowtokens = tokenize(tokens[5], '-');

        if (coltokens.size() != 2 || rowtokens.size() != 2)
            continue;

        int mask_colmin = std::stoi(coltokens[0]);
        int mask_colmax = std::stoi(coltokens[1]);
        int mask_rowmin = std::stoi(rowtokens[0]);
        int mask_rowmax = std::stoi(rowtokens[1]);

        if (_kmeansMasks.count(chroc) == 0)
        {
            _kmeansMasks[chroc] = {0, PLTU::LASTROW, 0, PLTU::LASTCOL};
        }

        if (mask_colmin != 0 && mask_colmin < _kmeansMasks[chroc][3])
            _kmeansMasks[chroc][3] = mask_colmin - 1;
        if (mask_rowmin != 0 && mask_rowmin < _kmeansMasks[chroc][1])
            _kmeansMasks[chroc][1] = mask_rowmin - 1;

        if (mask_colmax != PLTU::LASTCOL && mask_colmax > _kmeansMasks[chroc][2])
            _kmeansMasks[chroc][2] = mask_colmax + 1;
        if (mask_rowmax != PLTU::LASTROW && mask_rowmax > _kmeansMasks[chroc][0])
            _kmeansMasks[chroc][0] = mask_rowmax + 1;
    }

    return _kmeansMasks;
}
// Add to ask for the Slink file as an argument
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "No Slink .root was provided!" << std::endl;
        std::cout << "Usage: ./main [input folderpath] [slink filepath]" << std::endl;
        exit(1);
    }
    KmeansAnalyzer A;

    std::string inputFolderPath = argv[1];

    std::string centroidsFile = inputFolderPath + "/centroids_dummy.csv";
    std::string normalizationFile = inputFolderPath + "/normalization_dummy.csv";
    std::string maskFilePath = inputFolderPath + "/interface_conf_Mask_2018_24x36center_26x38outer.txt";

    std::map<std::pair<unsigned, unsigned>, std::vector<int>> maskEdges = readMaskFile(maskFilePath);
    for (auto const &m : maskEdges)
        std::cout << "Mask ch" << m.first.first << " ROC" << m.first.second << " : " << m.second[0] << " " << m.second[1] << " " << m.second[2] << " " << m.second[3] << " || " << m.second[1] - m.second[0] << " " << m.second[3] - m.second[2] << std::endl;

    A.initKmeansAnalyzer(centroidsFile, normalizationFile, maskEdges);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    std::string slinkFileName = argv[2];
    TFile *f = new TFile(slinkFileName.c_str());

    if (!f->IsOpen())
    {
        std::cout << "Cannot open file" << std::endl;
        exit(1);
    }

    TList *list = f->GetListOfKeys();
    TIter next(list);
    TKey *key;
    TObject *obj;

    while ((key = (TKey *)next()))
    {
        obj = key->ReadObj();
        std::string name = key->GetName();
        if (strcmp(obj->IsA()->GetName(), "TH2F") == 0 && name.rfind("Occupancy Ch20 ROC0 76738787 79495413", 0) == 0)
        {

            std::cout << "Reading Occupancy TH2F: " << name << std::endl;

            std::vector<int> spaceIndices;
            int j = 0;
            for (int i = 0; i < 3; i++)
            {
                while (name[j] != ' ')
                    j++;
                spaceIndices.push_back(j);
                j++;
            }
            int channel = std::stoi(name.substr(spaceIndices[0] + 3, spaceIndices[1] - (spaceIndices[0] + 3)));
            int ROC = std::stoi(name.substr(spaceIndices[1] + 4, spaceIndices[2] - (spaceIndices[1] + 4)));
            std::cout << "channel: " << channel << " ROC: " << ROC << std::endl;
            std::pair<int, int> chroc = std::make_pair(channel, ROC);

            TH2F *h = (TH2F *)obj;

            Int_t nbinsx = h->GetNbinsX();
            Int_t nbinsy = h->GetNbinsY();

            std::cout << "nbinsx: " << nbinsx << std::endl;
            std::cout << "nbinsy: " << nbinsy << std::endl;

            std::vector<std::vector<int>> matrix(nbinsy, std::vector<int>(nbinsx, 0));

            for (int a = 0; a < nbinsx; a++)
            {
                for (int b = 0; b < nbinsy; b++)
                {
                    matrix[b][a] = h->GetBinContent(h->FindBin(a, b));
                }
            }

            int classIndex = A.getClassIndex(matrix, maskEdges[chroc]);
            std::cout << "classIndex: " << classIndex << std::endl;

            std::cout << "isNormal: " << A.getIsNormal(classIndex) << std::endl;

            double fractionOfHitsOutsideOfActiveArea = A.getFractionOfHitsOutsideOfActiveArea(matrix, maskEdges[chroc]);
            std::cout << "fractionOfHitsOutsideOfActiveArea: " << fractionOfHitsOutsideOfActiveArea << std::endl;

            break;
        }
        // else std::cout<<name<<" is not a 2D histogram."<<std::endl;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    std::cout << "Done!" << std::endl;

    return 0;
}