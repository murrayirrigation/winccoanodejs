// Our Resources administration
#include  "ManagerResources.hxx"
#include  <ErrHdl.hxx>

// Wrapper to read config file
void  ManagerResources::init(int& argc, char* argv[])
{
    begin(argc, argv);
    while (readSection() || generalSection())
        ;
    end(argc, argv);
}


// Read the config file.
// Our section is [NodeJS] or [NodeJS<num>],
PVSSboolean  ManagerResources::readSection()
{
    // Is it our section ?
    // This will test for [NodeJS] and [NodeJS_<num>]
    if (!isSection("NodeJS"))
        return PVSS_FALSE;

    // Read next entry
    getNextEntry();

    // Loop thru section
    while ((getCfgState() != CFG_SECT_START) &&  // Not next section
        (getCfgState() != CFG_EOF))          // End of config file
    {
        if (!readGeneralKeyWords())            // keywords handled in Resources
        {
            ErrHdl::error(ErrClass::PRIO_WARNING,     // not that bad
                ErrClass::ERR_PARAM,        // wrong parametrization
                ErrClass::ILLEGAL_KEYWORD,  // illegal Keyword in Res.
                getKeyWord());

            // Signal error, so we stop later
            setCfgError();
        }

        getNextEntry();
    }

    // So the loop will stop at the end of the file
    return getCfgState() != CFG_EOF;
}

