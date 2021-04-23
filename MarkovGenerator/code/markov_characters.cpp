#include "markov.h"
#include "random.h"

struct markov_trie
{
    u32 TotalCount;
    u32 FrequencyTable[128];
    struct markov_trie *NextCharacter[128];
};

inline void
PushGram(struct memory_arena *Arena, struct markov_trie *MarkovTrie,
         char *Gram, char NextChar, u32 NGramSize)
{
    struct markov_trie *Node = MarkovTrie;
    for( ; NGramSize; --NGramSize, Node = Node->NextCharacter[*Gram], ++Gram)
    {
        if(Node->NextCharacter[*Gram] == 0)
        {
            Node->NextCharacter[*Gram] = PushStruct(Arena, struct markov_trie);
        }
    }

    ++Node->TotalCount;
    ++Node->FrequencyTable[NextChar];
}

inline char
GetCharacterForGram(struct markov_trie *MarkovTrie, char *Gram, u32 NGramSize)
{
    char Result = 0;

    for( ; NGramSize && MarkovTrie; --NGramSize, ++Gram)
    {
        MarkovTrie = MarkovTrie->NextCharacter[*Gram];
    }

    if(MarkovTrie && MarkovTrie->TotalCount)
    {
        u32 FrequencyCount = 0;
        for(u32 CharacterIndex = 0; CharacterIndex < ArrayCount(MarkovTrie->FrequencyTable); ++CharacterIndex)
        {
            u32 Count = *(MarkovTrie->FrequencyTable + CharacterIndex);
            if(Count)
            {
                FrequencyCount += Count;
                if((RandomNumber() % FrequencyCount) <= Count)
                {
                    Result = CharacterIndex;
                }
            }
        }
    }

    return(Result);
}

static inline void
StringNCopy(char *Dest, char *Src, u32 Length)
{
    while(Length--)
    {
        *Dest++ = *Src++;
    }
}

int
main(s32 ArgCount, char **Args)
{
    char *FileName = 0;
    u32 NGramSize = 7;
    s32 TargetLength = 256;
    char *UserSeedString = 0;
    if(ArgCount >= 5)
    {
        FileName = Args[1];
        NGramSize = atoi(Args[2]);
        TargetLength = atoi(Args[3]);

        s32 SeedStringLength = StringLength(Args[4]);
        if(SeedStringLength && (SeedStringLength == NGramSize))
        {
            UserSeedString = Args[4];
        }

        if(ArgCount == 6)
        {
            GlobalPRNG = atoi(Args[5]);
        }
        else
        {
            srand(time(0));
            GlobalPRNG = rand();
        }
    }
    else
    {
        printf("Usage: %s [source] [ngram length] [max output size] [seed string] <rng seed>\n", Args[0]);
        printf("   source         : Corpus to generate text from\n"
               "   ngram length   : Length of chain used to determine next characters\n"
               "   max output size: Maximum character length of generated output\n"
               "   seed string    : String to start markov chain generator from or \"\" a random seed\n"
               "                    Must be as long as ngram length value if supplied\n"
               "   rng seed       : (optional) Seed the random number generator\n");
        return(1);
    }

    FILE *File = fopen(FileName, "rb");
    if(!File)
    {
        fprintf(stderr, "Failed to open file %s\n", FileName);
        return(1);
    }

    fseek(File, 0, SEEK_END);
    u32 FileSize = ftell(File);
    fseek(File, 0, SEEK_SET);

    char *Text = (char *)malloc(sizeof(*Text) * FileSize);
    fread(Text, FileSize, 1, File);
    fclose(File);

    struct memory_arena Arena = InitializeArena(Megabytes(10));

    struct markov_trie *MarkovTrie = PushStruct(&Arena, struct markov_trie);

    char *At = Text;
    char *NextLetter = At + NGramSize;
    for(u32 NGramsToParse = FileSize - NGramSize;
        NGramsToParse;
        --NGramsToParse)
    {
        PushGram(&Arena, MarkovTrie, At, *NextLetter, NGramSize);
        ++At, ++NextLetter;
    }

    char *Output = PushArray(&Arena, Kilobytes(1), char);
    char *SeedStart = UserSeedString;
    if(!SeedStart)
    {
        SeedStart = Text + (RandomNumber() % (FileSize - NGramSize));
    }

    StringNCopy(Output, SeedStart, NGramSize);
    char *OutAt = Output;
    char *NextAt = Output + NGramSize;
    while(TargetLength--)
    {
        char NextLetter = GetCharacterForGram(MarkovTrie, OutAt++, NGramSize);
        *NextAt++ = NextLetter;

        if(NextLetter == 0)
        {
            break;
        }
    }

    printf("%s\n", Output);

    return(0);
}
