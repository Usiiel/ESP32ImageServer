FROM mcr.microsoft.com/dotnet/core/aspnet:2.1
COPY bin/Debug/netcoreapp2.1/publish/ App/
WORKDIR /App

EXPOSE 62100/tcp
EXPOSE 62101/tcp

ENV ASPNETCORE_URLS=http://*:62100

ENTRYPOINT ["dotnet", "ImageServer.dll"]