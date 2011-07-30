/*
 *
 *                This source code is part of
 *
 *                 G   R   O   M   A   C   S
 *
 *          GROningen MAchine for Chemical Simulations
 *
 * Written by David van der Spoel, Erik Lindahl, Berk Hess, and others.
 * Copyright (c) 1991-2000, University of Groningen, The Netherlands.
 * Copyright (c) 2001-2009, The GROMACS development team,
 * check out http://www.gromacs.org for more information.

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * If you want to redistribute modifications, please consider that
 * scientific software is very special. Version control is crucial -
 * bugs must be traceable. We will be happy to consider code for
 * inclusion in the official distribution, but derived work must not
 * be called official GROMACS. Details are found in the README & COPYING
 * files - if they are missing, get the official version at www.gromacs.org.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the papers on the package - you can find them in the top README file.
 *
 * For more info, check our website at http://www.gromacs.org
 */
/*! \internal \file
 * \brief
 * Tests option assignment.
 *
 * In addition to testing gmx::OptionsAssigner, these are the main
 * tests for the classes from basicoptions.h and basicoptionstorage.h (and
 * their base classes) that actually implement the behavior, as well as for the
 * internal implementation of the gmx::Options and gmx::AbstractOptionStorage
 * classes.
 *
 * \author Teemu Murtola <teemu.murtola@cbr.su.se>
 * \ingroup module_options
 */
#include <vector>

#include <gtest/gtest.h>

#include "gromacs/fatalerror/exceptions.h"
#include "gromacs/options/basicoptions.h"
#include "gromacs/options/options.h"
#include "gromacs/options/optionsassigner.h"

namespace
{

TEST(OptionsAssignerTest, HandlesMissingRequiredParameter)
{
    gmx::Options options(NULL, NULL);
    int value = 0;
    using gmx::IntegerOption;
    ASSERT_NO_THROW(options.addOption(
                        IntegerOption("p").store(&value).required()));

    EXPECT_THROW(options.finish(), gmx::InvalidInputError);
}

TEST(OptionsAssignerTest, HandlesInvalidMultipleParameter)
{
    gmx::Options options(NULL, NULL);
    std::vector<int> values;
    using gmx::IntegerOption;
    ASSERT_NO_THROW(options.addOption(
                        IntegerOption("p").storeVector(&values).multiValue()));

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("p"));
    ASSERT_NO_THROW(assigner.appendValue("1"));
    ASSERT_NO_THROW(assigner.finishOption());
    EXPECT_THROW(assigner.startOption("p"), gmx::InvalidInputError);
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());
}

TEST(OptionsAssignerTest, HandlesMultipleParameter)
{
    gmx::Options options(NULL, NULL);
    std::vector<int> values;
    using gmx::IntegerOption;
    ASSERT_NO_THROW(options.addOption(
                        IntegerOption("p").storeVector(&values).allowMultiple()));

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("p"));
    ASSERT_NO_THROW(assigner.appendValue("1"));
    EXPECT_NO_THROW(assigner.finishOption());
    ASSERT_NO_THROW(assigner.startOption("p"));
    ASSERT_NO_THROW(assigner.appendValue("2"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_TRUE(options.isSet("p"));
    ASSERT_EQ(2U, values.size());
    EXPECT_EQ(1, values[0]);
    EXPECT_EQ(2, values[1]);
}

TEST(OptionsAssignerTest, HandlesMissingValue)
{
    gmx::Options options(NULL, NULL);
    int value1 = 0, value2 = 0;
    using gmx::IntegerOption;
    ASSERT_NO_THROW(options.addOption(IntegerOption("p").store(&value1)));
    ASSERT_NO_THROW(options.addOption(IntegerOption("q").store(&value2)));

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("p"));
    EXPECT_THROW(assigner.finishOption(), gmx::InvalidInputError);
    ASSERT_NO_THROW(assigner.startOption("q"));
    ASSERT_NO_THROW(assigner.appendValue("2"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());
}

TEST(OptionsAssignerTest, HandlesExtraValue)
{
    gmx::Options options(NULL, NULL);
    int value1 = 0;
    using gmx::IntegerOption;
    ASSERT_NO_THROW(options.addOption(IntegerOption("p").store(&value1)));

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("p"));
    ASSERT_NO_THROW(assigner.appendValue("2"));
    EXPECT_THROW(assigner.appendValue("3"), gmx::InvalidInputError);
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());
}

TEST(OptionsAssignerTest, HandlesSubSections)
{
    gmx::Options options(NULL, NULL);
    gmx::Options sub1("section1", NULL);
    gmx::Options sub2("section2", NULL);
    int value = 3;
    int value1 = 1;
    int value2 = 2;
    using gmx::IntegerOption;
    ASSERT_NO_THROW(options.addOption(IntegerOption("p").store(&value)));
    ASSERT_NO_THROW(sub1.addOption(IntegerOption("p").store(&value1)));
    ASSERT_NO_THROW(sub2.addOption(IntegerOption("p").store(&value2)));
    ASSERT_NO_THROW(options.addSubSection(&sub1));
    ASSERT_NO_THROW(options.addSubSection(&sub2));

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startSubSection("section1"));
    ASSERT_NO_THROW(assigner.startOption("p"));
    EXPECT_NO_THROW(assigner.appendValue("5"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finishSubSection());
    ASSERT_NO_THROW(assigner.startOption("p"));
    EXPECT_NO_THROW(assigner.appendValue("4"));
    EXPECT_NO_THROW(assigner.finishOption());
    ASSERT_NO_THROW(assigner.startSubSection("section2"));
    ASSERT_NO_THROW(assigner.startOption("p"));
    EXPECT_NO_THROW(assigner.appendValue("6"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finishSubSection());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_EQ(4, value);
    EXPECT_EQ(5, value1);
    EXPECT_EQ(6, value2);
}

TEST(OptionsAssignerTest, HandlesNoStrictSubSections)
{
    gmx::Options options(NULL, NULL);
    gmx::Options sub1("section1", NULL);
    gmx::Options sub2("section2", NULL);
    int pvalue = 3;
    int pvalue1 = 1;
    int qvalue  = 4;
    int pvalue2 = 2;
    int rvalue  = 5;
    using gmx::IntegerOption;
    ASSERT_NO_THROW(options.addOption(IntegerOption("p").store(&pvalue)));
    ASSERT_NO_THROW(sub1.addOption(IntegerOption("p").store(&pvalue1)));
    ASSERT_NO_THROW(sub1.addOption(IntegerOption("q").store(&qvalue)));
    ASSERT_NO_THROW(sub2.addOption(IntegerOption("p").store(&pvalue2)));
    ASSERT_NO_THROW(sub2.addOption(IntegerOption("r").store(&rvalue)));
    ASSERT_NO_THROW(options.addSubSection(&sub1));
    ASSERT_NO_THROW(options.addSubSection(&sub2));

    gmx::OptionsAssigner assigner(&options);
    assigner.setNoStrictSectioning(true);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("q"));
    EXPECT_NO_THROW(assigner.appendValue("6"));
    EXPECT_NO_THROW(assigner.finishOption());
    ASSERT_NO_THROW(assigner.startOption("p"));
    EXPECT_NO_THROW(assigner.appendValue("7"));
    EXPECT_NO_THROW(assigner.finishOption());
    ASSERT_NO_THROW(assigner.startOption("r"));
    EXPECT_NO_THROW(assigner.appendValue("8"));
    EXPECT_NO_THROW(assigner.finishOption());
    ASSERT_NO_THROW(assigner.startOption("p"));
    EXPECT_NO_THROW(assigner.appendValue("9"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finishSubSection());
    ASSERT_NO_THROW(assigner.startOption("p"));
    EXPECT_NO_THROW(assigner.appendValue("10"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_EQ(6, qvalue);
    EXPECT_EQ(7, pvalue1);
    EXPECT_EQ(8, rvalue);
    EXPECT_EQ(9, pvalue2);
    EXPECT_EQ(10, pvalue);
}

TEST(OptionsAssignerTest, HandlesMultipleSources)
{
    gmx::Options options(NULL, NULL);
    int value = -1;
    using gmx::IntegerOption;
    ASSERT_NO_THROW(options.addOption(IntegerOption("p").store(&value)));

    {
        gmx::OptionsAssigner assigner(&options);
        EXPECT_NO_THROW(assigner.start());
        ASSERT_NO_THROW(assigner.startOption("p"));
        EXPECT_NO_THROW(assigner.appendValue("1"));
        EXPECT_NO_THROW(assigner.finishOption());
        EXPECT_NO_THROW(assigner.finish());
    }
    {
        gmx::OptionsAssigner assigner2(&options);
        EXPECT_NO_THROW(assigner2.start());
        ASSERT_NO_THROW(assigner2.startOption("p"));
        EXPECT_NO_THROW(assigner2.appendValue("2"));
        EXPECT_NO_THROW(assigner2.finishOption());
        EXPECT_NO_THROW(assigner2.finish());
    }
    EXPECT_NO_THROW(options.finish());

    EXPECT_EQ(2, value);
}


TEST(OptionsAssignerBooleanTest, StoresYesValue)
{
    gmx::Options options(NULL, NULL);
    bool  value = false;
    using gmx::BooleanOption;
    ASSERT_NO_THROW(options.addOption(BooleanOption("p").store(&value)));

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("p"));
    EXPECT_NO_THROW(assigner.appendValue("yes"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_TRUE(value);
}

TEST(OptionsAssignerBooleanTest, SetsBooleanWithoutExplicitValue)
{
    gmx::Options options(NULL, NULL);
    bool value = false;
    using gmx::BooleanOption;
    ASSERT_NO_THROW(options.addOption(BooleanOption("p").store(&value)));

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("p"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_TRUE(value);
}

TEST(OptionsAssignerBooleanTest, ClearsBooleanWithPrefixNo)
{
    gmx::Options options(NULL, NULL);
    bool value = true;
    using gmx::BooleanOption;
    ASSERT_NO_THROW(options.addOption(BooleanOption("p").store(&value)));

    gmx::OptionsAssigner assigner(&options);
    assigner.setAcceptBooleanNoPrefix(true);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("nop"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_FALSE(value);
}

TEST(OptionsAssignerBooleanTest, HandlesBooleanWithPrefixAndValue)
{
    gmx::Options options(NULL, NULL);
    bool value = false;
    using gmx::BooleanOption;
    ASSERT_NO_THROW(options.addOption(BooleanOption("p").store(&value)));

    gmx::OptionsAssigner assigner(&options);
    assigner.setAcceptBooleanNoPrefix(true);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("nop"));
    // It's OK to fail, but if it doesn't, it should work.
    try
    {
        assigner.appendValue("no");
        assigner.finishOption();
        EXPECT_NO_THROW(assigner.finish());
        EXPECT_TRUE(value);
    }
    catch (gmx::InvalidInputError &)
    {
    }
}


TEST(OptionsAssignerIntegerTest, StoresSingleValue)
{
    gmx::Options options(NULL, NULL);
    int value = 1;
    using gmx::IntegerOption;
    ASSERT_NO_THROW(options.addOption(IntegerOption("p").store(&value)));

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("p"));
    ASSERT_NO_THROW(assigner.appendValue("3"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_EQ(3, value);
}

TEST(OptionsAssignerIntegerTest, StoresDefaultValue)
{
    gmx::Options options(NULL, NULL);
    int value = -1;
    using gmx::IntegerOption;
    ASSERT_NO_THROW(options.addOption(
                        IntegerOption("p").store(&value).defaultValue(2)));
    EXPECT_EQ(2, value);

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_EQ(2, value);
}

TEST(OptionsAssignerIntegerTest, StoresDefaultValueIfSet)
{
    gmx::Options options(NULL, NULL);
    int value = -1;
    using gmx::IntegerOption;
    ASSERT_NO_THROW(options.addOption(
                        IntegerOption("p").store(&value).defaultValueIfSet(2)));
    EXPECT_EQ(-1, value);

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("p"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_EQ(2, value);
}

TEST(OptionsAssignerIntegerTest, HandlesDefaultValueIfSetWhenNotSet)
{
    gmx::Options options(NULL, NULL);
    int value = -1;
    using gmx::IntegerOption;
    ASSERT_NO_THROW(options.addOption(
                        IntegerOption("p").store(&value).defaultValueIfSet(2)));
    EXPECT_EQ(-1, value);

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_EQ(-1, value);
}

TEST(OptionsAssignerIntegerTest, HandlesBothDefaultValues)
{
    gmx::Options options(NULL, NULL);
    int value = -1;
    using gmx::IntegerOption;
    ASSERT_NO_THROW(options.addOption(
                        IntegerOption("p").store(&value)
                            .defaultValue(1).defaultValueIfSet(2)));
    EXPECT_EQ(1, value);

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("p"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_EQ(2, value);
}

TEST(OptionsAssignerIntegerTest, StoresToVector)
{
    gmx::Options          options(NULL, NULL);
    std::vector<int>      values;
    using gmx::IntegerOption;
    ASSERT_NO_THROW(options.addOption(
                        IntegerOption("p").storeVector(&values).multiValue()));

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("p"));
    ASSERT_NO_THROW(assigner.appendValue("-2"));
    ASSERT_NO_THROW(assigner.appendValue("1"));
    ASSERT_NO_THROW(assigner.appendValue("4"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_EQ(3U, values.size());
    EXPECT_EQ(-2, values[0]);
    EXPECT_EQ(1, values[1]);
    EXPECT_EQ(4, values[2]);
}

TEST(OptionsAssignerIntegerTest, HandlesVectors)
{
    gmx::Options options(NULL, NULL);
    int  vec[3] = {0, 0, 0};
    using gmx::IntegerOption;
    ASSERT_NO_THROW(options.addOption(IntegerOption("p").store(vec).vector()));

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("p"));
    ASSERT_NO_THROW(assigner.appendValue("-2"));
    ASSERT_NO_THROW(assigner.appendValue("1"));
    ASSERT_NO_THROW(assigner.appendValue("4"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_EQ(-2, vec[0]);
    EXPECT_EQ(1, vec[1]);
    EXPECT_EQ(4, vec[2]);
}

TEST(OptionsAssignerIntegerTest, HandlesVectorFromSingleValue)
{
    gmx::Options options(NULL, NULL);
    int  vec[3] = {0, 0, 0};
    using gmx::IntegerOption;
    ASSERT_NO_THROW(options.addOption(IntegerOption("p").store(vec).vector()));

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("p"));
    ASSERT_NO_THROW(assigner.appendValue("2"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_EQ(2, vec[0]);
    EXPECT_EQ(2, vec[1]);
    EXPECT_EQ(2, vec[2]);
}

TEST(OptionsAssignerIntegerTest, HandlesVectorsWithDefaultValue)
{
    gmx::Options options(NULL, NULL);
    int  vec[3] = {3, 2, 1};
    using gmx::IntegerOption;
    ASSERT_NO_THROW(options.addOption(IntegerOption("p").store(vec).vector()));

    EXPECT_NO_THROW(options.finish());

    EXPECT_EQ(3, vec[0]);
    EXPECT_EQ(2, vec[1]);
    EXPECT_EQ(1, vec[2]);
}


TEST(OptionsAssignerDoubleTest, StoresSingleValue)
{
    gmx::Options options(NULL, NULL);
    double value = 0.0;
    using gmx::DoubleOption;
    ASSERT_NO_THROW(options.addOption(DoubleOption("p").store(&value)));

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("p"));
    ASSERT_NO_THROW(assigner.appendValue("2.7"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_DOUBLE_EQ(2.7, value);
}


TEST(OptionsAssignerStringTest, StoresSingleValue)
{
    gmx::Options           options(NULL, NULL);
    std::string            value;
    using gmx::StringOption;
    ASSERT_NO_THROW(options.addOption(StringOption("p").store(&value)));

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("p"));
    ASSERT_NO_THROW(assigner.appendValue("value"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_EQ("value", value);
}

TEST(OptionsAssignerStringTest, HandlesEnumValue)
{
    gmx::Options           options(NULL, NULL);
    std::string            value;
    const char * const     allowed[] = { "none", "test", "value", NULL };
    int                    index = -1;
    using gmx::StringOption;
    ASSERT_NO_THROW(options.addOption(
                        StringOption("p").store(&value)
                            .enumValue(allowed).storeEnumIndex(&index)));

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("p"));
    ASSERT_NO_THROW(assigner.appendValue("test"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_EQ("test", value);
    EXPECT_EQ(1, index);
}

TEST(OptionsAssignerStringTest, HandlesIncorrectEnumValue)
{
    gmx::Options           options(NULL, NULL);
    std::string            value;
    const char * const     allowed[] = { "none", "test", "value", NULL };
    int                    index = -1;
    using gmx::StringOption;
    ASSERT_NO_THROW(options.addOption(
                        StringOption("p").store(&value)
                            .enumValue(allowed).storeEnumIndex(&index)));

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("p"));
    ASSERT_THROW(assigner.appendValue("unknown"), gmx::InvalidInputError);
}

TEST(OptionsAssignerStringTest, CompletesEnumValue)
{
    gmx::Options           options(NULL, NULL);
    std::string            value;
    const char * const     allowed[] = { "none", "test", "value", NULL };
    int                    index = -1;
    using gmx::StringOption;
    ASSERT_NO_THROW(options.addOption(
                        StringOption("p").store(&value)
                            .enumValue(allowed).storeEnumIndex(&index)));

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    ASSERT_NO_THROW(assigner.startOption("p"));
    ASSERT_NO_THROW(assigner.appendValue("te"));
    EXPECT_NO_THROW(assigner.finishOption());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_EQ("test", value);
    EXPECT_EQ(1, index);
}

TEST(OptionsAssignerStringTest, HandlesEnumWithNoValue)
{
    gmx::Options           options(NULL, NULL);
    std::string            value;
    const char * const     allowed[] = { "none", "test", "value", NULL };
    int                    index = -3;
    using gmx::StringOption;
    ASSERT_NO_THROW(options.addOption(
                        StringOption("p").store(&value)
                            .enumValue(allowed).storeEnumIndex(&index)));
    EXPECT_TRUE(value.empty());
    EXPECT_EQ(-1, index);

    ASSERT_NO_THROW(options.finish());

    EXPECT_TRUE(value.empty());
    EXPECT_EQ(-1, index);
}

TEST(OptionsAssignerStringTest, HandlesEnumDefaultValue)
{
    gmx::Options           options(NULL, NULL);
    std::string            value;
    const char * const     allowed[] = { "none", "test", "value", NULL };
    int                    index = -1;
    using gmx::StringOption;
    ASSERT_NO_THROW(options.addOption(
                        StringOption("p").store(&value)
                            .enumValue(allowed).defaultValue("test")
                            .storeEnumIndex(&index)));
    EXPECT_EQ("test", value);
    EXPECT_EQ(1, index);

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_EQ("test", value);
    EXPECT_EQ(1, index);
}

TEST(OptionsAssignerStringTest, HandlesEnumDefaultIndex)
{
    gmx::Options           options(NULL, NULL);
    std::string            value;
    const char * const     allowed[] = { "none", "test", "value", NULL };
    int                    index = -1;
    using gmx::StringOption;
    ASSERT_NO_THROW(options.addOption(
                        StringOption("p").store(&value)
                            .enumValue(allowed).defaultEnumIndex(1)
                            .storeEnumIndex(&index)));
    EXPECT_EQ("test", value);
    EXPECT_EQ(1, index);

    gmx::OptionsAssigner assigner(&options);
    EXPECT_NO_THROW(assigner.start());
    EXPECT_NO_THROW(assigner.finish());
    EXPECT_NO_THROW(options.finish());

    EXPECT_EQ("test", value);
    EXPECT_EQ(1, index);
}

} // namespace
