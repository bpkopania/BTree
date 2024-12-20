#include "Student.h"

void Student::print()
{
	printf_s("%-30s %4d %4d %4d %5.2f\n", name, kol1, kol2, kol3, avg());
}

bool Student::isGreater(Entity& other)
{
	Student* otherStudent = dynamic_cast<Student*>(&other);
	if (otherStudent) {
		return this->avg() >= otherStudent->avg();
	}
	return false;
}

bool Student::equal(Entity& other)
{
	Student* otherStudent = dynamic_cast<Student*>(&other);
	if (otherStudent) {
		return this->avg() == otherStudent->avg();
	}
	return false;
}

bool Student::isSmaller(Entity& other)
{
	Student* otherStudent = dynamic_cast<Student*>(&other);
	if (otherStudent) {
		return this->avg() < otherStudent->avg();
	}
	return false;
}


float Student::avg() const
{
	return (kol1 + kol2 + kol3) / 3.0;
}

Student::Student(char* name, short kol1, short kol2, short kol3)
	: kol1(kol1), kol2(kol2), kol3(kol3)
{
	strncpy_s(this->name, name, 30);
}

Student::Student()
{
	char tempName[30] = "\0";
	strncpy_s(this->name, tempName, 30);
	kol1 = 0;
	kol2 = 0;
	kol3 = 0;
}

bool Student::isValid()
{
	return (kol1 >= 1 && kol1 <= 6) &&
		(kol2 >= 1 && kol2 <= 6) &&
		(kol3 >= 1 && kol3 <= 6) &&
		(strcmp(name, "\0") != 0);
}

