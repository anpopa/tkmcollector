/*-
 * Copyright (c) 2020 Alin Popa
 * All rights reserved.
 */

/*
 * @author Alin Popa <alin.popa@fxdata.ro>
 */

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

#include "../source/Application.h"
#include "gtest/gtest.h"

using namespace std;
using namespace cds::server;

class GTestApplication : public ::testing::Test
{
protected:
  GTestApplication();
  virtual ~GTestApplication();

  virtual void SetUp();
  virtual void TearDown();

protected:
  unique_ptr<Application> m_app;
};

GTestApplication::GTestApplication()
{
  m_app = make_unique<Application>(
      "CDSS", "CDS Server Application", cdsDefaults.getFor(Defaults::Default::ConfPath));
}

GTestApplication::~GTestApplication() {}

void GTestApplication::SetUp() {}

void GTestApplication::TearDown() {}

TEST_F(GTestApplication, printConfig) {}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
