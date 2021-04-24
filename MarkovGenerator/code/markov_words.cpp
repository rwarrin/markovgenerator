#include "markov.h"
#include "random.h"

inline u32
StringNWordCount(char *Source)
{
    u32 Result = 0;

    for(;;)
    {
        if(IsWhitespaceOrNull(*Source))
        {
            ++Result;
            ConsumeWhitespace(&Source);

            if(*Source == 0)
            {
                break;
            }
        }

        ++Source;
    }

    return(Result);
}

inline s32
StringNWordCompare(char *A, char *B, u32 Words = 1)
{
    for( ; *A == *B; ++A, ++B)
    {
        if(IsWhitespaceOrNull(*A) && (--Words == 0))
        {
            return 0;
        }
    }

    return(*A - *B);
}

static inline void
Swap(char **A, char **B)
{
    char *Temp = *A;
    *A = *B;
    *B = Temp;
}

static inline void
QuickSort(char **Array, s32 Lower, s32 Upper, u32 WordSize)
{
    if(Lower >= Upper)
    {
        return;
    }

    char *PartitionValue = Array[Upper];
    s32 PartitionAt = Lower - 1;

    for(s32 Index = Lower; Index < Upper; ++Index)
    {
        if(StringNWordCompare(Array[Index], PartitionValue, WordSize) < 0)
        {
            Swap(&Array[++PartitionAt], &Array[Index]);
        }
    }
    Swap(&Array[++PartitionAt], &Array[Upper]);

    QuickSort(Array, Lower, PartitionAt - 1, WordSize);
    QuickSort(Array, PartitionAt + 1, Upper, WordSize);
}

static inline u32
StringCopyNWords(char *Dest, char *Src, u32 Words = 1)
{
    u32 Result = 0;

    while(Words)
    {
        if(IsWhitespaceOrNull(*Src))
        {
            --Words;
        }

        *Dest++ = *Src++;
        ++Result;
    }

    return(Result);
}

int
main(s32 ArgCount, char **Args)
{
    char *FileName = 0;
    u32 NGramSize = 2;
    s32 TargetWords = 256;
    char *UserSeedString = 0;
    if(ArgCount >= 5)
    {
        FileName = Args[1];
        NGramSize = atoi(Args[2]);
        TargetWords = atoi(Args[3]);

        s32 SeedStringWordCount = StringNWordCount(Args[4]);
        if(SeedStringWordCount && (SeedStringWordCount == NGramSize))
        {
            UserSeedString = Args[4];
            u32 SeedLength = StringLength(UserSeedString);
            if(UserSeedString[SeedLength - 1] != ' ')
            {
                UserSeedString[SeedLength] = ' ';
            }
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
               "   ngram length   : Length of word chain used to determine next characters\n"
               "   max output size: Maximum word count of generated output\n"
               "   seed string    : String to start markov chain generator from or \"\" a random seed\n"
               "                    Must have as many words as [ngram length]\n"
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
    u32 FileLength = ftell(File);
    fseek(File, 0, SEEK_SET);

    char *Text = (char *)malloc(sizeof(Text)*FileLength + 1);
    fread(Text, FileLength, 1, File);
    fclose(File);
    Text[FileLength] = 0;

    u32 WordCount = 0;
    char *ParseAt = Text;
    while((ParseAt - Text) < FileLength)
    {
        ConsumeWhitespace(&ParseAt);
        if(*ParseAt == 0)
        {
            break;
        }

        ++WordCount;
        while(!IsWhitespace(*ParseAt))
        {
            ++ParseAt;
        }
    }

    struct memory_arena Arena = InitializeArena(Megabytes(5));

    u32 NGramCount = 0;
    char **NGrams = PushArray(&Arena, WordCount, char *);
    ParseAt = Text;
    while((ParseAt - Text) < FileLength)
    {
        ConsumeWhitespace(&ParseAt);
        if(*ParseAt == 0)
        {
            break;
        }

        *(NGrams + NGramCount++) = ParseAt;
        while(!IsWhitespace(*ParseAt))
        {
            ++ParseAt;
        }
    }
    Assert(NGramCount == WordCount);

    QuickSort(NGrams, 0, NGramCount - 1, NGramSize);

#if 0
    for(u32 Index = 0; Index < NGramCount - 2; ++Index)
    {
        Assert(StringNWordCompare(NGrams[Index], NGrams[Index + 1], NGramSize) <= 0);
    }
#endif

    char *Seed = UserSeedString;
    if(!Seed)
    {
        Seed = *(NGrams + (RandomNumber() % (NGramCount)));
    }

    u32 OutputBufferLength = Megabytes(3);
    char *OutputBuffer = PushArray(&Arena, OutputBufferLength, char);

    char *Phrase = OutputBuffer;
    u32 LastPhraseLength = StringCopyNWords(Phrase, Seed, NGramSize);
    while(TargetWords > 0)
    {
        u32 Lower = 0;
        u32 Upper = NGramCount - 1;
        while(Lower < Upper)
        {
            u32 Middle = (Lower + Upper) / 2;
            s32 CompareResult = StringNWordCompare(Phrase, NGrams[Middle], NGramSize);
            if(CompareResult == 0)
            {
                Upper = Middle;
            }
            else if(CompareResult < 0)
            {
                Upper = Middle - 1;
            }
            else
            {
                Lower = Middle + 1;
            }
        }

        u32 FirstMatchingGramIndex = Upper;
        while((FirstMatchingGramIndex > 0) &&
              (StringNWordCompare(Phrase, NGrams[FirstMatchingGramIndex - 1], NGramSize) == 0))
        {
            --FirstMatchingGramIndex;
        }

        char *NextGram = 0;
        for(u32 SearchIndex = 0;
            ((FirstMatchingGramIndex + SearchIndex) < NGramCount) &&
            StringNWordCompare(Phrase, NGrams[FirstMatchingGramIndex + SearchIndex], NGramSize) == 0;
            ++SearchIndex)
        {
            if(RandomNumber() % (SearchIndex + 1) == 0)
            {
                NextGram = NGrams[FirstMatchingGramIndex + SearchIndex];
            }
        }

        if(!NextGram || *NextGram == 0)
        {
            break;
        }

        char *NextGramAt = NextGram;
        for(u32 Skip = NGramSize; Skip; ++NextGramAt)
        {
            if(*NextGramAt == 0)
            {
                break;
            }

            if(IsWhitespace(*NextGramAt))
            {
                --Skip;
            }
        }
        ConsumeWhitespace(&NextGramAt);

        if(*NextGramAt == 0)
        {
            break;
        }

        Phrase = Phrase + LastPhraseLength;
        LastPhraseLength = StringCopyNWords(Phrase, NextGramAt, NGramSize);
        if(((Phrase + LastPhraseLength) - OutputBuffer) >= OutputBufferLength)
        {
            break;
        }
        TargetWords -= NGramSize;
    }

    printf("%s\n", OutputBuffer);

    return(0);
}
